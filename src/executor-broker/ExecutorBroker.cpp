#include <brokers/ExecutorBroker.h>

#include <brokers/ServiceBroker.h>
#include <interprocess-communication/InterprocessServer.h>
#include <interprocess-communication/common/ConnectionValidator.h>

#include <shared-services/SharedServiceExport.h>

#include <core/AbstractFactory.h>
#include <core/Encoding.h>

#include <api/services/IServiceRouter.h>
#include <api/services/ServiceInfo.h>

namespace Gengine {
using namespace AppConfig;
using namespace Services;
using namespace InterprocessCommunication;

namespace Services {

namespace {
IServiceRouterPtr GetServiceRouter() {
  return std::dynamic_pointer_cast<IServiceRouter>(GetRouter());
}
PipeConnection GeneratePipeConnection(const std::string& key) {
  PipeConnection connection;

  static int count = 0;
  connection.pipe = std::wstring(L"") + utf8toWchar(key) + L"_" +
                    std::to_wstring(++count) + L"_generated";
  return connection;
}
}  // namespace

using TFactory =
    AbstractFactory::AbstractFactory<InterprocessCommunication::InterfaceImpl,
                                     std::string,
                                     const interface_key&,
                                     IMicroService&>;

class ExecutorBroker : public IExecutorBroker, public Worker {
 public:
  ExecutorBroker();
  ~ExecutorBroker();

  void Configure(
      const std::unordered_map<std::string, ServiceConfig>& config) override;
  void Register(const std::string& key, TExecutorCreator&& creator) override;
  void Unregister(const std::string& key) override;
  bool Run(const std::string& id, IMicroService& handler) override;
  void Stop(const std::string& id) override;

 private:
  struct context {
    context(ExecutorBroker& parent,
            const std::string& id,
            const std::string& key)
        : id(id), key(key), parent(parent) {}
    virtual ~context() = default;
    std::string id;
    std::string key;
    ExecutorBroker& parent;
    std::uint32_t connectionTimerId = InvalidTimerID;

    virtual bool Run(const std::string& id, IMicroService& handler) = 0;
    virtual void Stop() = 0;
  };
  struct localContext : context {
    class ClientCreator
        : public AbstractFactory::IAbstractCreator<IMicroService,
                                                   const interface_key&> {
     public:
      ClientCreator(IMicroService& handler) : handler(handler) {}
      std::shared_ptr<IMicroService> Create(
          const interface_key& key) const override {
        return std::shared_ptr<IMicroService>(&handler, [](void*) {});
      }

     private:
      IMicroService& handler;
    };

    localContext(ExecutorBroker& parent,
                 const std::string& id,
                 const std::string& key)
        : context(parent, id, key) {}
    std::unique_ptr<StaticServiceRegistrator> registrator;

    bool Run(const std::string& id, IMicroService& handler) {
      if (!registrator) {
        registrator = std::make_unique<StaticServiceRegistrator>(
            key, std::make_shared<ClientCreator>(handler), ServiceType::Local);
      }
      registrator->Do();
      return true;
    }
    void Stop() {
      if (registrator) {
        registrator->Undo();
      }
    }
  };
  friend localContext;
  struct dynamicInterprocessContext : localContext {
    dynamicInterprocessContext(ExecutorBroker& parent,
                               const std::string& id,
                               const std::string& key,
                               std::uint32_t threadId)
        : localContext(parent, id, key), threadId(threadId) {}

    bool Run(const std::string& id, IMicroService& handler) override {
      localContext::Run(id, handler);

      if (!executor) {
        executor = parent.Factory.Create(key, id, handler);
      }

      if (executor) {
        if (!server) {
          auto connection = GetConnection();
          if (boost::apply_visitor(ConnectionValidator(), connection)) {
            std::shared_ptr<InterprocessServer> existingServer;
            auto iter = std::find_if(
                parent.signals.begin(), parent.signals.end(),
                [&existingServer, connection,
                 this](const std::map<std::string, std::unique_ptr<context>>::
                           value_type& other) {
                  auto ctx = dynamic_cast<dynamicInterprocessContext*>(
                      other.second.get());
                  if (ctx) {
                    auto result =
                        connection == ctx->GetConnection() && ctx->server;
                    if (result) {
                      existingServer = ctx->server;
                      return true;
                    }
                  }
                  return false;
                });
            if (iter != parent.signals.end()) {
              server = existingServer;
            } else {
              server =
                  std::make_shared<InterprocessServer>(connection, threadId);
            }
          }
        }

        if (server) {
          server->AddExecutor(executor);
          server->Start();
          return true;
        }
      }
      return false;
    }

    void Stop() override {
      if (server) {
        auto thatIter = std::find_if(
            parent.signals.begin(), parent.signals.end(),
            [this](
                const std::map<std::string,
                               std::unique_ptr<context>>::value_type& other) {
              auto ctx =
                  dynamic_cast<dynamicInterprocessContext*>(other.second.get());
              if (ctx) {
                return this != ctx && GetConnection() == ctx->GetConnection() &&
                       ctx->server;
              }
              return false;
            });
        if (thatIter == parent.signals.end()) {
          server->Stop();
        }
        server->RemoveExecutor(executor);
      }
    }

    virtual ipc_connection GetConnection() {
      if (!info) {
        auto router = GetServiceRouter();
        if (router) {
          info = std::make_shared<ServiceInfo>();
          info->id = id;
          info->key = key;
          info->type = ServiceType::Remote;
          info->pipe = GeneratePipeConnection(key).pipe;
          router->RegisterService(utf8toWchar(id), *info);
        } else {
          return ipc_connection();
        }
      }
      PipeConnection connection;
      connection.pipe = info->pipe;
      return connection;
    }

    std::shared_ptr<ServiceInfo> info;
    const std::uint32_t threadId;
    std::shared_ptr<InterprocessServer> server;
    std::shared_ptr<InterfaceImpl> executor;
  };
  friend dynamicInterprocessContext;
  struct interprocessContext : dynamicInterprocessContext {
    interprocessContext(ExecutorBroker& parent,
                        const std::string& id,
                        const std::string& key,
                        const ipc_connection& data,
                        std::uint32_t threadId)
        : dynamicInterprocessContext(parent, id, key, threadId), data(data) {}

    ipc_connection GetConnection() override { return data; }

    ipc_connection data;
  };
  friend interprocessContext;
  std::map<std::string, std::unique_ptr<context>> signals;
  TFactory Factory;
};

ExecutorBroker::ExecutorBroker() : Worker(13) {
  ThreadConfig config;
  config.id = 13;
  config.name = "ExecutorBrokerThread";
  GENGINE_REGISTER_THREAD(config);
}

ExecutorBroker::~ExecutorBroker() {
  for (const auto& signalIter : signals) {
    if (signalIter.second->connectionTimerId != InvalidTimerID) {
      GENGINE_STOP_TIMER(signalIter.second->connectionTimerId);
    }
  }
}

void ExecutorBroker::Configure(
    const std::unordered_map<std::string, ServiceConfig>& config) {
  struct ServiceContextCreator
      : public boost::static_visitor<std::unique_ptr<context>> {
    ServiceContextCreator(ExecutorBroker& parent, const ServiceConfig& config)
        : parent(parent), config(config) {}
    std::unique_ptr<context> operator()(const boost::blank&) const {
      return std::make_unique<localContext>(parent, config.id, config.key);
    }
    std::unique_ptr<context> operator()(const ExternalConnection&) const {
      return std::make_unique<dynamicInterprocessContext>(
          parent, config.id, config.key, config.threadRef.get());
    }
    std::unique_ptr<context> operator()(
        const SharedConnection& connection) const {
      return nullptr;
    }
    std::unique_ptr<context> operator()(
        const InterprocessCommunication::ipc_connection& connection) const {
      return std::make_unique<interprocessContext>(
          parent, config.id, config.key, connection, config.threadRef.get());
    }

   private:
    ExecutorBroker& parent;
    const ServiceConfig& config;
  };

  for (const auto& service : config) {
    auto context =
        boost::apply_visitor(ServiceContextCreator(*this, service.second),
                             service.second.connection);
    if (context) {
      signals.emplace(std::make_pair(service.first, std::move(context)));
    }
  }

  GENGINE_ENABLE_EXECUTORS
}

void ExecutorBroker::Register(const std::string& key,
                              TExecutorCreator&& creator) {
  Factory.RegisterCreator(key, std::move(creator));
}

void ExecutorBroker::Unregister(const std::string& key) {
  Factory.RemoveCreator(key);
}

bool ExecutorBroker::Run(const std::string& id, IMicroService& handler) {
  auto iter = signals.find(id);
  if (iter != signals.end()) {
    auto& context = iter->second;

    auto task = [&context, id, &handler] { return context->Run(id, handler); };

    if (!task()) {
      if (context->connectionTimerId == InvalidTimerID) {
        context->connectionTimerId =
            GENGINE_START_TIMER(task, std::chrono::seconds{1});
      }
    }
    return true;
  }
  return false;
}

void ExecutorBroker::Stop(const std::string& id) {
  auto iter = signals.find(id);
  if (iter != signals.end()) {
    iter->second->Stop();

    if (iter->second->connectionTimerId != InvalidTimerID)
      GENGINE_STOP_TIMER_WITH_WAIT(iter->second->connectionTimerId);
  }
}

EXPORT_GLOBAL_SHARED_SERVICE(ExecutorBroker)

}  // namespace Services
}  // namespace Gengine
