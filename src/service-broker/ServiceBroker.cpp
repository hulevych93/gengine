#include <brokers/ServiceBroker.h>

#include <functional>
#include <unordered_map>
#include <boost/signals2.hpp>

#include <core/Encoding.h>
#include <appconfig/AppConfig.h>
#include <brokers/WorkerBroker.h>

#include <brokers/ProcessBroker.h>
#include <interprocess-communication/InterprocessClientInterface.h>

#include <shared-services/SharedServiceExport.h>
#include <shared-services/ISharedLibraryClient.h>

#include <api/services/IServiceRouter.h>
#include <api/services/ServiceInfo.h>

#include <services/SessionQuery.h>

namespace Gengine {
using namespace InterprocessSynchronization;
using namespace InterprocessCommunication;
using namespace SharedServices;
using namespace AppConfig;

namespace Services {

TIServiceRouter GetServiceRouter()
{
    return std::dynamic_pointer_cast<IServiceRouter>(GetRouter());
}

using TFactory = AbstractFactory<IMicroService, std::string, const interface_key&>;
using TFactories = std::unordered_map<ServiceType, TFactory>;
using service_signal = boost::signals2::signal<void(const TService&)>;

class ServiceBroker: public IServiceBroker
    , public Worker
    , public IServiceClient
{
public:
   ServiceBroker();
   ~ServiceBroker();

   void Configure(const std::unordered_map<std::string, ServiceConfig>& inServices, const std::unordered_map<std::string, ServiceConfig>& outServices) override;
   void Deconfigure() override;
   void Register(const std::string& id, TServiceCreator&& creator, ServiceType preffered) override;
   void Unregister(const std::string& id, ServiceType preffered) override;
   void Touch(const std::string& id, IServiceClient& client) override;
   connection Subscribe(const std::string& id, IServiceClient& client) override;
   void Kill(const std::string& id) override;

protected:
    void OnServiceAvailable(const TService& service) override;
    void OnServiceUnavailable(const TService& service) override;

private:
    static TFactories Factories;

private:
   struct ServiceContextBase: public Worker
   {
      ServiceContextBase()
          : Worker(2)
      {}
      virtual ~ServiceContextBase() = default;

      virtual connection Subscribe(IServiceClient& client)
      {
          connection conns;
          conns.emplace_back(connected.connect([&client](const TService& service) { client.OnServiceAvailable(service); }));
          conns.emplace_back(disconnected.connect([&client](const TService& service) { client.OnServiceUnavailable(service); }));
          return conns;
      }

      virtual bool IsConnected() const
      {
          return service != nullptr;
      }

      virtual void Touch(IServiceClient& client) = 0;
      virtual bool Connect() = 0;
      virtual void Kill() = 0;

      std::string id;
      std::string key;
      TService service;
      service_signal connected;
      service_signal disconnected;
   };

   struct ServiceContext : ServiceContextBase,
                           public IProcessClient
   {
       ~ServiceContext()
       {
           if (connectionTimerId != INVALID_TIMER_ID)
           {
               GENGINE_STOP_TIMER(connectionTimerId);
           }
       }

       void OnProcessLauched() override
       {
           Connect();
       }

       void OnProcessStopped() override
       {
           if (service)
           {
               disconnected(service);
               service.reset();
           }
       }

       void Touch(IServiceClient& client) override
       {
           if (!service)
           {
               if (!processHolder)
               {
                   Connect();
               }
               else
               {
                   processHolder->Run();
               }
           }
           else
           {
               client.OnServiceAvailable(service);
           }
       }

       void Kill() override
       {
           if (connectionTimerId != INVALID_TIMER_ID)
               GENGINE_STOP_TIMER_WITH_WAIT(connectionTimerId);

           if (processHolder)
               processHolder->Stop();

           if (service)
           {
               disconnected(service);
               service.reset();
           }
       }

       bool Connect() override
       {
           if (!service)
           {
               auto factoryIter = Factories.find(type);
               if (factoryIter != Factories.end())
               {
                   auto& factory = factoryIter->second;
                   service = factory.Create(key, id);
               }
           }

           struct ServiceConnector : public boost::static_visitor<bool>
           {
               ServiceConnector(ServiceContext& context)
                   : m_context(context)
               {}
               bool operator()(const boost::blank&) const
               {
                   return m_context.service != nullptr;
               }
               bool operator()(const ExternalConnection&) const
               {
                   auto router = GetServiceRouter();
                   if (router)
                   {
                       auto available = false;
                       router->IsServiceAvailable(utf8toWchar(m_context.id), &available);
                       if (available)
                       {
                           ServiceInfo info;
                           if (router->GetService(utf8toWchar(m_context.id), &info))
                           {
                               service_connection data;
                               PipeConnection connection;
                               connection.pipe = info.pipe;
                               data = connection;
                               return boost::apply_visitor(*this, data);
                           }
                       }
                   }
                   return false;
               }
               bool operator()(const SharedConnection& connection) const
               {
                   auto sharedService = std::dynamic_pointer_cast<ISharedLibraryClient>(m_context.service);
                   if (sharedService)
                   {
                       m_context.service = sharedService->Connect(connection);
                   }
                   return m_context.service != nullptr;
               }
               bool operator()(const ipc_connection& connection) const
               {
                   auto interprocessService = std::dynamic_pointer_cast<InterprocessClientInterface>(m_context.service);
                   if (interprocessService)
                   {
                       return interprocessService->Connect(connection);
                   }
                   return false;
               }

           private:
               ServiceContext & m_context;
           };

           auto task = [this]() mutable {
               if (boost::apply_visitor(ServiceConnector(*this), data))
               {
                   connected(service);
                   if (connectionTimerId != INVALID_TIMER_ID)
                   {
                       GENGINE_STOP_TIMER(connectionTimerId);
                       connectionTimerId = INVALID_TIMER_ID;
                   }
                   return true;
               }
               else if (!required)
               {
                   if (triesCount < 3)
                   {
                       ++triesCount;
                   }
                   else if(triesCount == 3)
                   {
                       assert(triesCount == 3);

                       auto factoryIter = Factories.find(ServiceType::Null);
                       assert(factoryIter != Factories.end());

                       auto& factory = factoryIter->second;
                       auto service = factory.Create(key, id);
                       assert(service);

                       connected(service);
                   }
               }

               return false;
           };

           if (!task())
           {
               if (connectionTimerId == INVALID_TIMER_ID)
               {
                   connectionTimerId = GENGINE_START_TIMER(task, 1000);
               }

               return false;
           }

           return true;
       }

       ServiceType type;
       service_connection data;
       std::unique_ptr<ProcessHolder> processHolder;
       std::uint32_t connectionTimerId = INVALID_TIMER_ID;
       std::uint32_t triesCount = 0;
       bool required = false;
   };

   struct CompositeServiceContext : ServiceContextBase,
                                    public IServiceClient
   {
       connection Subscribe(IServiceClient& client) override
       {
           for (const auto& context : contexts)
           {
               auto conns = context->Subscribe(*this);
               for (auto& conn : conns)
               {
                   connections.push_back(std::move(conn));
               }
           }
           return ServiceContextBase::Subscribe(client);
       }

       void Touch(IServiceClient& client) override
       {
           if (!service)
           {
               Connect();

               for (const auto& context : contexts)
               {
                   context->Touch(*this);
               }
           }
           else
           {
               client.OnServiceAvailable(service);
           }
       }

       void Kill() override
       {
           for (const auto& context : contexts)
           {
               context->Kill();
           }

           service.reset();
       }

   private:
       void OnServiceAvailable(const TService& thatService) override
       {
           auto client = GetCompositeClient();
           const auto isConnected = client->Count() == 0;
           client->RegisterService(thatService);
           if (isConnected)
           {
               connected(service);
           }
       }

       void OnServiceUnavailable(const TService& thatService) override
       {
           auto client = GetCompositeClient();
           client->UnregisterService(thatService);
           if (client->Count() == 0)
           {
               disconnected(service);
           }
       }
       
       bool Connect() override
       {
           if (!service)
           {
               auto factoryIter = Factories.find(ServiceType::Composite);
               if (factoryIter != Factories.end())
               {
                   auto& factory = factoryIter->second;
                   service = factory.Create(key, id);
                   assert(service);
               }
           }

           return true;
       }

       bool IsConnected() const override
       {
           auto client = GetCompositeClient();
           const auto isConnected = client->Count() != 0;
           return isConnected;
       }

   public:
       std::shared_ptr<ICompositeClient> GetCompositeClient() const
       {
           return std::dynamic_pointer_cast<ICompositeClient>(service);
       }

       std::vector<std::shared_ptr<ServiceContext>> contexts;
       connection connections;
   };

   std::unordered_map<std::string, std::shared_ptr<ServiceContextBase>> signals;

private:
    const TService& GetRouter() override;

private:
    connection m_routerConnection;
    mutable TService m_router;
    static const std::string RouterId;
};

const std::string ServiceBroker::RouterId("622E9F42");

void ServiceBroker::OnServiceAvailable(const TService& service)
{
    m_router = service;
}

void ServiceBroker::OnServiceUnavailable(const TService& service)
{
    m_router.reset();
}

const TService& ServiceBroker::GetRouter()
{
    if (!m_router && m_routerConnection.empty())
    {
        m_routerConnection = Subscribe(RouterId, *this);
        Touch(RouterId, *this);
    }
    return m_router;
}

template<class Interface, class Implementation, class... Args>
class LocalConcreteCreator : public IAbstractCreator<Interface, Args...>
{
public:
    std::shared_ptr<Interface> Create(Args... args) const override
    {
        return std::make_shared<Implementation>();
    }
};

TFactories ServiceBroker::Factories = {
   { ServiceType::Local, TFactory {
   std::make_pair("ISessionQuery", std::make_shared<LocalConcreteCreator<IMicroService, SessionQuery, const interface_key&>>())
   }
   },
{ ServiceType::Remote, TFactory() },
{ ServiceType::Shared, TFactory() },
{ ServiceType::Composite, TFactory() },
{ ServiceType::Null, TFactory() }
};

ServiceBroker::ServiceBroker()
    : Worker(2)
{
    {
        ThreadConfig config;
        config.id = 2;
        config.name = "ServiceBrokerThread";
        GENGINE_REGISTER_THREAD(config);
    }

    {
        ThreadConfig config;
        config.id = 73;
        config.name = "ClientsThread";
        GENGINE_REGISTER_THREAD(config);
    }

    {
        ThreadConfig config;
        config.id = 1;
        config.name = "ExecutableLauncherThread";
        GENGINE_REGISTER_THREAD(config);
    }

    {
        ThreadConfig config;
        config.id = 222;
        config.name = "ServiceClientStubThread";
        GENGINE_REGISTER_THREAD(config);
    }

    {
        auto serviceContext = std::make_shared<ServiceContext>();
        serviceContext->id = "3514D35D";
        serviceContext->key = "ISessionQuery";
        serviceContext->type = ServiceType::Local;
        signals.emplace(std::make_pair(serviceContext->id, std::move(serviceContext)));
    }

    {
        auto serviceContext = std::make_shared<ServiceContext>();
        serviceContext->id = "48B156CF";
        serviceContext->key = "IEnvironment";
        serviceContext->type = ServiceType::Local;
        signals.emplace(std::make_pair(serviceContext->id, std::move(serviceContext)));
    }
}

ServiceBroker::~ServiceBroker()
{
}

void ServiceBroker::Configure(const std::unordered_map<std::string, ServiceConfig>& inServices, const std::unordered_map<std::string, ServiceConfig>& outServices)
{
    auto task = [this, inServices, outServices]() {
        for (const auto& serviceIter : inServices)
        {
            auto& serviceConfig = serviceIter.second;

            if (serviceConfig.type == ServiceType::Composite)
            {
                auto compositeContext = std::make_shared<CompositeServiceContext>();

                auto& serviceConfig = serviceIter.second;
                compositeContext->id = serviceConfig.id;
                compositeContext->key = serviceConfig.key;

                for (const auto& ref : serviceConfig.serviceRefs)
                {
                    auto serviceFound = signals.find(ref);
                    if (serviceFound != signals.end())
                    {
                        compositeContext->contexts.emplace_back(std::static_pointer_cast<ServiceContext>(serviceFound->second));
                    }
                }
                signals.emplace(std::make_pair(serviceIter.first, std::move(compositeContext)));
            }
            else
            {
                auto serviceContext = std::make_shared<ServiceContext>();

                serviceContext->id = serviceConfig.id;
                serviceContext->key = serviceConfig.key;
                serviceContext->type = serviceConfig.type;
                serviceContext->data = serviceConfig.connection;
                if (serviceConfig.processRef)
                {
                    serviceContext->processHolder = std::make_unique<ProcessHolder>(serviceConfig.processRef.get(), serviceContext.get());
                }
                if (serviceConfig.required)
                {
                    serviceContext->required = serviceConfig.required.get();
                }

                signals.emplace(std::make_pair(serviceIter.first, std::move(serviceContext)));
            }
        }

        for (const auto& serviceIter : outServices)
        {
            auto& serviceConfig = serviceIter.second;
            auto serviceContext = std::make_shared<ServiceContext>();
            serviceContext->id = serviceConfig.id;
            serviceContext->key = serviceConfig.key;
            serviceContext->type = ServiceType::Local;
            signals.emplace(std::make_pair(serviceIter.first, std::move(serviceContext)));
        }
    };
    GENGINE_POST_WAITED_TASK(task);
    GENGINE_ENABLE_SERVICES
}

void ServiceBroker::Deconfigure()
{
    auto task = [this]() {
        for (const auto& signal : signals)
        {
            Kill(signal.first);
        }
        signals.clear();
    };
    GENGINE_POST_DEINITIALIZATION_TASK(task);
    GENGINE_DISABLE_SERVICES
}

void ServiceBroker::Register(const std::string& key, TServiceCreator&& creator, ServiceType preffered)
{
    auto& factory = Factories.at(preffered);
    factory.RegisterCreator(key, creator);
}

void ServiceBroker::Unregister(const std::string& key, ServiceType preffered)
{
    auto& factory = Factories.at(preffered);
    factory.RemoveCreator(key);
}

connection ServiceBroker::Subscribe(const std::string& id, IServiceClient& client)
{
   connection conns;

   auto task = [this, id, &client, &conns]() {
       auto iter = signals.find(id);
       if (iter != signals.end())
       {
           auto& context = iter->second;
           conns = context->Subscribe(client);
       }
   };
   GENGINE_POST_WAITED_TASK(task);

   return conns;
}

void ServiceBroker::Touch(const std::string& id, IServiceClient& client)
{
    auto task = [this, id, &client]() {
        auto iter = signals.find(id);
        if (iter != signals.end())
        {
            auto& context = iter->second;
            context->Touch(client);
        }
    };
    GENGINE_POST_TASK(task);
}

void ServiceBroker::Kill(const std::string& id)
{
    auto task = [this, id]() {
        auto iter = signals.find(id);
        if (iter != signals.end())
        {
            auto& context = iter->second;
            context->Kill();
        }
    };
    GENGINE_POST_WAITED_TASK(task);
}

EXPORT_GLOBAL_SHARED_SERVICE(ServiceBroker)

}
}
