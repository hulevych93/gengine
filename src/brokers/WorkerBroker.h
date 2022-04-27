#pragma once

#include <memory>
#include <functional>
#include <appconfig/AppConfig.h>

namespace Gengine {
namespace Services {

using task_t = std::function<void()>;

class IFuture
{
public:
    virtual ~IFuture() = default;
    virtual void Wait(std::uint32_t timeout) = 0;
    virtual void Cancel() = 0;
    virtual bool IsCanceled() const = 0;
    virtual void Complete() = 0;
    virtual void Reset() = 0;
};

class IWorkerThread
{
public:
    virtual ~IWorkerThread() = default;
    virtual  void PostDeinializationTask(task_t task) = 0;
    virtual  std::shared_ptr<IFuture> PostTask(task_t task) = 0;
    virtual  void PostTaskAndWait(task_t task) = 0;

    virtual  std::int32_t StartTimer(task_t task, std::uint32_t intervalMS, std::uint32_t firstDelayMS = 0) = 0;
    virtual  std::shared_ptr<IFuture> StopTimer(std::int32_t timerId) = 0;
    virtual  void StopAndWaitTimer(std::int32_t timerId) = 0;

    virtual void ClearTasks() = 0;
    virtual void Dispose() = 0;
};

class IWorkerBroker
{
public:
    virtual ~IWorkerBroker() = default;
    virtual void Configure(const std::set<ThreadConfig>& config) = 0;
    virtual std::shared_ptr<IWorkerThread> GetThread(std::uint8_t id) = 0;
    virtual void Shutdown() = 0;
};

class Worker
{
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

static const std::int32_t INVALID_TIMER_ID = -1;

#define GENGINE_INTIALIZE_CONCURRENCY(config) InitializeConcurrency(config)
#define GENGINE_SHUTDOWN_CONCURRENCY ShutdownConcurrency();
#define GENGINE_REGISTER_THREAD(thread) \
std::set<ThreadConfig> threadConf = { thread };\
GENGINE_INTIALIZE_CONCURRENCY(threadConf)

#define POST_THREAD_TASK(THREAD, HANDLER) THREAD->PostTask(HANDLER);
#define POST_THREAD_WAITED_TASK(THREAD, HANDLER) THREAD->PostTaskAndWait(HANDLER);
#define POST_HEARTBEAT_TASK(HANDLER) GetWorkingThread() != nullptr ? \
                                 GetWorkingThread()->PostTask(HANDLER) : \
                                 std::shared_ptr<Gengine::Services::IFuture>()
#define POST_HEARTBEAT_WAITED_TASK(HANDLER) if(GetWorkingThread() != nullptr)\
                                         GetWorkingThread()->PostTaskAndWait(HANDLER);
#define START_THREAD_TIMER(THREAD, HANDLER, PERIOD) THREAD->StartTimer(HANDLER), PERIOD);
#define START_HEARTBEAT_TIMER(HANDLER, PERIOD) GetWorkingThread() != nullptr ? \
                                               GetWorkingThread()->StartTimer(HANDLER, PERIOD) : \
                                               Services::INVALID_TIMER_ID;
#define START_HEARTBEAT_TIMER_WITH_DELAY(HANDLER, PERIOD, OFFSET) GetWorkingThread() != nullptr ? \
                                               GetWorkingThread()->StartTimer(HANDLER, PERIOD, OFFSET) : \
                                               Services::INVALID_TIMER_ID;
#define STOP_HEARTBEAT_TIMER(TIMER_ID) GetWorkingThread()->StopTimer(TIMER_ID);
#define STOP_HEARTBEAT_TIMER_WITH_WAIT(TIMER_ID) GetWorkingThread()->StopAndWaitTimer(TIMER_ID);
#define POST_HEARTBEAT_DEINITIALIZATION_TASK(HANDLER) if(GetWorkingThread() != nullptr) \
                                 GetWorkingThread()->PostDeinializationTask(HANDLER);

}
}
