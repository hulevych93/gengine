#include <brokers/ProcessBroker.h>

#include <shared-services/SharedServiceImport.h>
#include <boost/signals2.hpp>

namespace Gengine {
using namespace AppConfig;
using namespace SharedServices;

namespace Services {

namespace {
std::shared_ptr<IProcessBroker> GetBroker() {
  static std::shared_ptr<IProcessBroker> ProcessBroker;
  if (!ProcessBroker) {
    SharedConnection connection;
    connection.path = "process-broker";
    connection.symbol = "ProcessBroker_service";
    ProcessBroker = import_symbol<IProcessBroker>(connection);
  }
  assert(ProcessBroker);
  return ProcessBroker;
}
}  // namespace

ProcessHolder::ProcessHolder(uint32_t id, IProcessClient* client)
    : m_id(id), m_listener(client) {}

void ProcessHolder::Run() {
  endpoint = GetBroker()->PowerUp(m_id, *this);
}

void ProcessHolder::Stop() {
  if (!endpoint.empty()) {
    GetBroker()->TearDown(m_id);
    endpoint.clear();
  }
}

ProcessHolder::~ProcessHolder() {
  Stop();
}

void ProcessHolder::OnProcessLauched() {
  if (m_listener)
    m_listener->OnProcessLauched();
}

void ProcessHolder::OnProcessStopped() {
  if (m_listener)
    m_listener->OnProcessStopped();
}

void InitializeProcesses(const std::set<ProcessConfig>& config) {
  GetBroker()->Configure(config);
}

void DeinitializeProcesses() {
  GetBroker()->Deconfigure();
}

}  // namespace Services
}  // namespace Gengine
