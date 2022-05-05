#pragma once

#include <appconfig/AppConfig.h>
#include <core/AbstractFactory.h>
#include <core/StaticList.h>

namespace Gengine {
namespace InterprocessCommunication {
class InterfaceImpl;
}
namespace Services {

class IMicroService;
using TExecutor = std::shared_ptr<InterprocessCommunication::InterfaceImpl>;
using TExecutorCreator = std::shared_ptr<AbstractFactory::IAbstractCreator<
    InterprocessCommunication::InterfaceImpl,
    const InterprocessCommunication::interface_key&,
    IMicroService&>>;

class IExecutorBroker {
 public:
  virtual ~IExecutorBroker() = default;
  virtual void Configure(
      const std::unordered_map<std::string, ServiceConfig>& config) = 0;
  virtual void Register(const std::string& key, TExecutorCreator&& creator) = 0;
  virtual void Unregister(const std::string& key) = 0;
  virtual bool Run(const std::string& id, IMicroService& handler) = 0;
  virtual void Stop(const std::string& id) = 0;
};

class ServiceObject {
 public:
  ~ServiceObject();

 protected:
  ServiceObject();

  bool Connect(const std::string& id, IMicroService& impl);
  void Disconnect();

 private:
  struct ServiceObjectImpl;
  std::unique_ptr<ServiceObjectImpl> m_impl;
};

template <class ServiceType>
class ServiceObjectProxy : public ServiceObject {
 public:
  template <class... Args>
  ServiceObjectProxy(const std::string& id, Args&&... args)
      : m_service(std::forward<Args>(args)...), m_id(id), m_connected(false) {}
  ~ServiceObjectProxy() { Hide(); }

  bool Reveal() {
    if (!m_connected) {
      m_connected = Connect(m_id, m_service);
    }
    return m_connected;
  }

  void Hide() {
    if (m_connected) {
      Disconnect();
      m_connected = false;
    }
  }

 private:
  ServiceType m_service;
  const std::string m_id;
  bool m_connected;
};

template <class Type>
class ServiceObjectProxy<std::shared_ptr<Type>> : public ServiceObject {
 public:
  template <class... Args>
  ServiceObjectProxy(const std::string& id, std::shared_ptr<Type>&& service)
      : m_service(std::move(service)), m_id(id), m_connected(false) {}

  bool Reveal() {
    if (!m_connected) {
      m_connected = Connect(m_id, *m_service);
    }
    return m_connected;
  }

  void Hide() {
    if (m_connected) {
      Disconnect();
      m_connected = false;
    }
  }

 private:
  std::shared_ptr<Type> m_service;
  const std::string m_id;
  bool m_connected;
};

class StaticExecutorRegistrator {
 public:
  StaticExecutorRegistrator(std::string key, TExecutorCreator&& creator);

  void Do();
  void Undo();

 private:
  std::string m_key;
  TExecutorCreator m_creator;
};

void InitializeExecutors(
    const std::unordered_map<std::string, ServiceConfig>& config);

#define GENGINE_REGISTER_EXECUTOR(key, handler, name) \
  static FactoryItem<StaticExecutorRegistrator> const \
      registrationBroker##name(StaticExecutorRegistrator(key, handler));

#define GENGINE_ENABLE_EXECUTORS              \
  do {                                        \
    Runtime<StaticExecutorRegistrator>::Do(); \
  } while (false);
#define GENGINE_DISABLE_EXECUTORS               \
  do {                                          \
    Runtime<StaticExecutorRegistrator>::Undo(); \
  } while (false);

#define GENGINE_INITIALIZE_EXECUTORS(config) \
  GENGINE_ENABLE_EXECUTORS InitializeExecutors(config)
#define GENGINE_UNGENGINE_INITIALIZE_EXECUTORS GENGINE_DISABLE_EXECUTORS

}  // namespace Services
}  // namespace Gengine
