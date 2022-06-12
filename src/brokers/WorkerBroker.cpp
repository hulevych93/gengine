#include <brokers/WorkerBroker.h>

#include <core/Logger.h>
#include <shared-services/SharedServiceImport.h>

namespace Gengine {
using namespace SharedServices;

namespace Services {

Worker::Worker(std::uint32_t id) : m_id(id) {}

Worker::~Worker() = default;

void Worker::Cancel() {
  Dispose();
}

void Worker::Dispose() {
  if (m_thread)
    m_thread->Dispose(std::chrono::seconds{0});
}

namespace {
std::shared_ptr<IWorkerBroker> GetBroker() {
  static std::shared_ptr<IWorkerBroker> WorkerBroker;
  if (!WorkerBroker) {
    SharedConnection data;
    data.path = "worker-broker";
    data.symbol = "WorkerBroker_service";
    WorkerBroker = import_symbol<IWorkerBroker>(data);
  }
  assert(WorkerBroker);
  return WorkerBroker;
}
}  // namespace

std::shared_ptr<IWorkerThread> Worker::GetWorkingThread() const {
  if (!m_thread) {
    m_thread = GetBroker()->GetThread(m_id);
  }
  return m_thread;
}

void InitializeConcurrency(const std::set<ThreadConfig>& config) {
  GetBroker()->Configure(config);
}

void ShutdownConcurrency() {
  GetBroker()->Shutdown();
}

}  // namespace Services
}  // namespace Gengine
