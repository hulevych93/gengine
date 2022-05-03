#pragma once

#include <semaphore.h>
#include <signal.h>
#include <atomic>
#include <thread>

#include <entries/SimpleExecutor.h>
#include <multithreading/Event.h>

namespace Gengine {
namespace Entries {
class DaemonExecutor : public SimpleExecutor {
 public:
  DaemonExecutor(const DaemonExecutor&) = delete;
  DaemonExecutor(IEntry& entry);
  ~DaemonExecutor() = default;

  bool Execute(void* args) override;
  bool CreateProcessors(
      std::vector<std::unique_ptr<ICmdProcessor>>* processors) override;

 protected:
  virtual void OnSignal(int signum);
  virtual void OnCrash(int signum);
  virtual void OnTerm();

 protected:
  static void SignalHandler(int iSigNum);
  void StartSignalThread();
  void StopSignalThread();
  void SignalHandlingRoutine();

 protected:
  static volatile sig_atomic_t m_terminationInProgress;

  static volatile sig_atomic_t m_handlingSignal;
  std::thread m_signalHandlerThread;
  std::atomic<bool> m_canHandleSignals;
  Multithreading::Event m_stopEvent;

 private:
  static DaemonExecutor* m_instance;

 private:
  sem_t m_signalSemaphore;
  int m_lastSigNum;

  // 200
  int m_crashBuffSize;
  int m_crashAddressesNum;
  void* m_crashBuff[200];
};
}  // namespace Entries
}  // namespace Gengine
