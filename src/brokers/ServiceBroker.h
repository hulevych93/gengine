#pragma once

#include <vector>
#include <mutex>
#include <memory>
#include <condition_variable>

#include <boost/signals2/connection.hpp>

#include <core/AbstractFactory.h>
#include <core/StaticList.h>
#include <core/IMicroService.h>
#include <appconfig/AppConfig.h>
#include <brokers/WorkerBroker.h>

namespace Gengine {
namespace Services {

class IMicroService;
using TService = std::shared_ptr<IMicroService>;
using TServiceCreator = std::shared_ptr<IAbstractCreator<IMicroService, const std::string&>>;

class ICompositeClient
{
public:
    virtual ~ICompositeClient() = default;
    virtual void RegisterService(const TService& service) = 0;
    virtual void UnregisterService(const TService& service) = 0;
    virtual std::size_t Count() = 0;
};

class IServiceClient
{
public:
   virtual ~IServiceClient() = default;
   virtual void OnServiceAvailable(const TService& service) = 0;
   virtual void OnServiceUnavailable(const TService& service) = 0;
};

using connection = std::vector<boost::signals2::scoped_connection>;

class IServiceBroker
{
public:
    virtual ~IServiceBroker() = default;
    virtual void Configure(const std::unordered_map<std::string, ServiceConfig>& inServices, const std::unordered_map<std::string, ServiceConfig>& outServices) = 0;
    virtual void Deconfigure() = 0;
    virtual void Register(const std::string& id, TServiceCreator&& creator, ServiceType preffered) = 0;
    virtual void Unregister(const std::string& id, ServiceType preffered) = 0;
    virtual void Touch(const std::string& id, IServiceClient& client) = 0;
    virtual connection Subscribe(const std::string& id, IServiceClient& client) = 0;
    virtual void Kill(const std::string& id) = 0;
    virtual const TService& GetRouter() = 0;
};

class ServiceClient : public IServiceClient
{
public:
   ~ServiceClient();

protected:
   ServiceClient();

   void Connect(const std::string& key);
   void Disconnect();

private:
   struct ServiceClientImpl;
   std::unique_ptr<ServiceClientImpl> m_impl;
};

class ServiceClientStub final : public ServiceClient,
                                public Worker
{
public:
    using TServiceCall = std::function<void(const TService& service)>;

public:
    ServiceClientStub(const std::string& key);
    void Execute(TServiceCall call);

protected:
    void OnServiceAvailable(const TService& service) override;
    void OnServiceUnavailable(const TService& service) override;

protected:
    bool InitService();
    void Run();

private:
    const std::string m_key;
    std::mutex m_serviceMtx;
    TService m_service;
    std::vector<TServiceCall> m_calls;
};

template<class ServiceType>
class ServiceClientProxy final: public ServiceClient
{
public:
   ServiceClientProxy(const std::string& key)
      : m_key(key)
   {}

   ServiceType* operator->()
   {
      InitService();
      if (!m_service)
          throw std::runtime_error((std::string("bad service call ") + m_key.c_str()).c_str());
      return m_service.operator->();
   }

   ServiceType* operator->() const
   {
      const_cast<ServiceClientProxy<ServiceType>*>(this)->InitService();
      if (!m_service)
          throw std::runtime_error((std::string("bad service call ") + m_key.c_str()).c_str());
      return m_service.operator->();
   }

   operator bool() const
   {
       return m_service != nullptr;
   }

protected:
   void OnServiceAvailable(const TService& service) override
   {
      std::unique_lock<std::mutex> locker(m_serviceMtx);
      m_service = std::dynamic_pointer_cast<ServiceType>(service);
      m_serviceCondition.notify_all();
   }

   void OnServiceUnavailable(const TService& service) override
   {
      std::unique_lock<std::mutex> locker(m_serviceMtx);
      m_service.reset();
   }

protected:
   void InitService()
   {
       std::unique_lock<std::mutex> locker(m_serviceMtx);
       if (!m_service)
       {
           Connect(m_key);
           m_serviceCondition.wait_for(locker, std::chrono::seconds(30), [this]() {
               return m_service != nullptr;
           });
       }
   }

private:
   const std::string m_key;
   std::mutex m_serviceMtx;
   std::shared_ptr<ServiceType> m_service;
   std::condition_variable m_serviceCondition;
};

class StaticServiceRegistrator
{
public:
    StaticServiceRegistrator(const std::string& key, TServiceCreator&& creator, ServiceType preffered = ServiceType::Local);

    void Do();
    void Undo();

private:
    std::string m_key;
    TServiceCreator m_creator;
    ServiceType m_preffered;
};

void InitializeServices(const std::unordered_map<std::string, ServiceConfig>& inServices, const std::unordered_map<std::string, ServiceConfig>& outServices);
void DeInitializeServices();
const TService& GetRouter();

#define REGISTER_SERVICE(key, handler, name, serviceType) \
    static FactoryItem<StaticServiceRegistrator> const registrationBroker##name(StaticServiceRegistrator(key, handler, serviceType));

#define ENABLE_SERVICES { Runtime<StaticServiceRegistrator>::Do(); } while(false);
#define DISABLE_SERVICES { Runtime<StaticServiceRegistrator>::Undo(); } while(false);

#define INITIALIZE_SERVICES(inServices, outServices) ENABLE_SERVICES InitializeServices(inServices, outServices)
#define UNINITIALIZE_SERVICES DeInitializeServices(); DISABLE_SERVICES

}
}

