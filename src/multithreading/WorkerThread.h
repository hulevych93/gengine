#pragma once

#include <atomic>
#include <functional>

#include <chrono>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <brokers/WorkerBroker.h>
#include <multithreading/Event.h>

namespace UnitTests {
class WorkerThreadTest;
}

namespace Gengine {
namespace Multithreading {

class WorkerThread final : public Services::IWorkerThread {
  friend class UnitTests::WorkerThreadTest;

 public:
  WorkerThread(const std::string& name);
  virtual ~WorkerThread();

  std::string GetID() const;

  std::shared_ptr<Services::IFuture> PostTask(Services::task_t task) override;
  void PostTaskAndWait(Services::task_t task) override;

  std::shared_ptr<Services::IFuture> PostTask(
      Services::cancelable_task_t task) override;
  void PostTaskAndWait(Services::cancelable_task_t task) override;

  std::int32_t StartTimer(
      Services::task_t task,
      std::chrono::system_clock::duration interval,
      std::chrono::system_clock::duration firstDelay = DontWait) override;
  std::shared_ptr<Services::IFuture> StopTimer(std::int32_t timerId) override;

  void Dispose(std::chrono::system_clock::duration timeout) override;

 private:
  void RunLoop();

 private:
  class Future;
  struct FutureLock;

  struct TaskInfo final {
    Services::task_t task;
    std::weak_ptr<Future> future;
  };

  struct TimerInfo final {
    std::shared_ptr<Future> future;
    Services::task_t task;
    std::uint32_t id;
    std::chrono::system_clock::duration interval;
    std::chrono::system_clock::time_point scheduledTime;
  };

 private:
  std::pair<const std::string, bool> m_name;
  mutable std::mutex m_mutex;
  Event m_tasksCondition;
  std::atomic<bool> m_canRun;
  std::chrono::system_clock::time_point m_disposeTime;

 private:
  using TimersType = std::vector<std::shared_ptr<TimerInfo>>;
  using TasksType = std::vector<TaskInfo>;

  TimersType m_timers;
  TasksType m_postedTasks;
  std::thread m_executorThread;

 public:
  static const std::int32_t InvalidTimerID = -1;
};

}  // namespace Multithreading
}  // namespace Gengine
