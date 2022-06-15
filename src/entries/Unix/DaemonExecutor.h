#pragma once

#include <atomic>

#include <entries/SimpleExecutor.h>

#include <multithreading/Event.h>
#include <multithreading/WorkerThread.h>

namespace Gengine {
namespace Entries {
class DaemonExecutor : public SimpleExecutor {
 public:
  DaemonExecutor(const DaemonExecutor&) = delete;
  DaemonExecutor(DaemonExecutor&&) = delete;
  DaemonExecutor(IEntry& entry);
  ~DaemonExecutor() = default;

  bool Execute(void* args) override;
  bool CreateProcessors(
      std::vector<std::unique_ptr<ICmdProcessor>>* processors) override;

 private:
  void OnSignal(int signum);

 private:
  static void SignalHandler(int signum);

 private:
  static DaemonExecutor* m_instance;

 private:
  Multithreading::Event m_stopEvent;
  std::unique_ptr<Multithreading::WorkerThread> m_signalWorker;
};
}  // namespace Entries
}  // namespace Gengine
