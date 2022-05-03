#include <brokers/ExecutorBroker.h>

#include <core/Logger.h>
#include <shared-services/SharedServiceImport.h>

namespace Gengine {
using namespace AppConfig;
using namespace SharedServices;

namespace Services {

namespace {
std::shared_ptr<IExecutorBroker> GetBroker() {
  static std::shared_ptr<IExecutorBroker> ExecutorBroker;
  if (!ExecutorBroker) {
    SharedConnection connection;
    connection.path = "executor-broker";
    connection.symbol = "ExecutorBroker_service";
    ExecutorBroker = import_symbol<IExecutorBroker>(connection);
  }
  assert(ExecutorBroker);
  return ExecutorBroker;
}
}  // namespace

void InitializeExecutors(
    const std::unordered_map<std::string, ServiceConfig>& config) {
  GetBroker()->Configure(config);
}

struct ServiceObject::ServiceObjectImpl {
  std::string id;
};

ServiceObject::ServiceObject()
    : m_impl(std::make_unique<ServiceObjectImpl>()) {}

ServiceObject::~ServiceObject() {
  Disconnect();
}

bool ServiceObject::Connect(const std::string& id, IMicroService& impl) {
  if (m_impl->id.empty()) {
    auto success = GetBroker()->Run(id, impl);
    if (success) {
      m_impl->id = id;
    }
    return success;
  } else
    return true;
}

void ServiceObject::Disconnect() {
  if (!m_impl->id.empty()) {
    GetBroker()->Stop(m_impl->id);
    m_impl->id.clear();
  }
}

StaticExecutorRegistrator::StaticExecutorRegistrator(std::string key,
                                                     TExecutorCreator&& creator)
    : m_key(key), m_creator(std::move(creator)) {}

void StaticExecutorRegistrator::Do() {
  GetBroker()->Register(m_key, std::move(m_creator));
}

void StaticExecutorRegistrator::Undo() {
  GetBroker()->Unregister(m_key);
}

}  // namespace Services
}  // namespace Gengine
