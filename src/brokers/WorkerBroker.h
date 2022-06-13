#pragma once

#include <appconfig/AppConfig.h>
#include <chrono>
#include <functional>
#include <memory>

namespace Gengine {
namespace Services {

using task_t = std::function<void()>;
using calcelled_callback = std::function<bool()>;
using cancelable_task_t = std::function<void(calcelled_callback)>;

constexpr std::chrono::system_clock::duration WaitInfinite =
    std::chrono::hours(24);
constexpr std::chrono::system_clock::duration DontWait =
    std::chrono::seconds(0);

class IFuture {
 public:
  virtual ~IFuture() = default;
  virtual void Wait(std::chrono::system_clock::duration timeout) = 0;
};

class IWorkerThread {
 public:
  virtual ~IWorkerThread() = default;

  virtual std::shared_ptr<IFuture> PostTask(task_t task) = 0;
  virtual void PostTaskAndWait(task_t task) = 0;

  virtual std::shared_ptr<IFuture> PostTask(cancelable_task_t task) = 0;
  virtual void PostTaskAndWait(cancelable_task_t task) = 0;

  virtual std::int32_t StartTimer(
      task_t task,
      std::chrono::system_clock::duration interval,
      std::chrono::system_clock::duration firstDelay =
          std::chrono::milliseconds(0)) = 0;
  virtual std::shared_ptr<IFuture> StopTimer(std::int32_t timerId) = 0;

  virtual void Dispose(std::chrono::system_clock::duration timeout) = 0;
};

class IWorkerBroker {
 public:
  virtual ~IWorkerBroker() = default;
  virtual void Configure(const std::set<ThreadConfig>& config) = 0;
  virtual std::shared_ptr<IWorkerThread> GetThread(std::uint8_t id) = 0;
  virtual void Shutdown() = 0;
};

class Worker {
 public:
  Worker(std::uint32_t key);
  ~Worker();

  std::shared_ptr<IWorkerThread> GetWorkingThread() const;

 protected:
  void Cancel();
  void Dispose();
  std::uint8_t m_id;
  mutable std::shared_ptr<IWorkerThread> m_thread;
};

void InitializeConcurrency(const std::set<ThreadConfig>& config);
void ShutdownConcurrency();

static const std::int32_t InvalidTimerID = -1;

#define GENGINE_INTIALIZE_CONCURRENCY(config) InitializeConcurrency(config)
#define GENGINE_SHUTDOWN_CONCURRENCY ShutdownConcurrency();
#define GENGINE_REGISTER_THREAD(thread)         \
  std::set<ThreadConfig> threadConf = {thread}; \
  GENGINE_INTIALIZE_CONCURRENCY(threadConf)

#define GENGINE_POST_THREAD_TASK(THREAD, HANDLER) THREAD->PostTask(HANDLER);
#define GENGINE_POST_THREAD_WAITED_TASK(THREAD, HANDLER) \
  THREAD->PostTaskAndWait(HANDLER);
#define GENGINE_POST_TASK(HANDLER)            \
  GetWorkingThread() != nullptr               \
      ? GetWorkingThread()->PostTask(HANDLER) \
      : std::shared_ptr<Gengine::Services::IFuture>()
#define GENGINE_POST_WAITED_TASK(HANDLER) \
  if (GetWorkingThread() != nullptr)      \
    GetWorkingThread()->PostTaskAndWait(HANDLER);
#define GENGINE_START_THREAD_TIMER(THREAD, HANDLER, PERIOD) THREAD->StartTimer(HANDLER), PERIOD);
#define GENGINE_START_TIMER(HANDLER, PERIOD)            \
  GetWorkingThread() != nullptr                         \
      ? GetWorkingThread()->StartTimer(HANDLER, PERIOD) \
      : Services::InvalidTimerID;
#define GENGINE_START_LOOP(HANDLER)                                           \
  GetWorkingThread() != nullptr                                               \
      ? GetWorkingThread()->StartTimer(HANDLER, std::chrono::milliseconds{0}) \
      : Services::InvalidTimerID;
#define GENGINE_START_TIMER_WITH_DELAY(HANDLER, PERIOD, OFFSET) \
  GetWorkingThread() != nullptr                                 \
      ? GetWorkingThread()->StartTimer(HANDLER, PERIOD, OFFSET) \
      : Services::InvalidTimerID;
#define GENGINE_STOP_TIMER(TIMER_ID) GetWorkingThread()->StopTimer(TIMER_ID);
#define GENGINE_STOP_TIMER_WITH_WAIT(TIMER_ID)           \
  {                                                      \
    auto wait = GetWorkingThread()->StopTimer(TIMER_ID); \
    if (wait)                                            \
      wait->Wait(Services::WaitInfinite);                \
  }

#define GENGINE_POST_DEINITIALIZATION_TASK(HANDLER)      \
  if (GetWorkingThread() != nullptr) {                   \
    GetWorkingThread()->PostTask(HANDLER);               \
    GetWorkingThread()->Dispose(Services::WaitInfinite); \
  }

}  // namespace Services
}  // namespace Gengine
