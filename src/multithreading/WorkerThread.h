#pragma once

#include <map>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <functional>

#include <multithreading/Event.h>
#include <brokers/WorkerBroker.h>

namespace Gengine {
namespace Multithreading {

class WorkerThread: public Services::IWorkerThread
{
public:
    WorkerThread(const std::wstring& name, bool canRun = true);
    virtual ~WorkerThread();

    std::string GetID() const;

    void PostDeinializationTask(Services::task_t task) override;
    std::shared_ptr<Services::IFuture> PostTask(Services::task_t task) override;
    void PostTaskAndWait(Services::task_t task) override;

    std::int32_t StartTimer(Services::task_t task, std::uint32_t intervalMS, std::uint32_t firstDelayMS = 0) override;
    std::shared_ptr<Services::IFuture> StopTimer(std::int32_t timerId) override;
    void StopAndWaitTimer(std::int32_t timerId) override;

protected:
    void Dispose() override;
    void ClearTasks() override;

    void Initialize();
    void ThreadProc();

private:
    std::unique_ptr<std::thread> m_executorThread;

    struct TaskInfo
    {
        Services::task_t task;
        std::shared_ptr<Services::IFuture> asyncResult;
    };

    struct TimerInfo : TaskInfo
    {
        TimerInfo()
            : intervalMS(0)
            , nextScheduledTime(0)
        {}

        std::int32_t intervalMS;
        std::uint32_t nextScheduledTime;
    };

private:
    std::pair<const std::wstring, bool> m_name;
    mutable std::mutex m_tasksMutex;
    Event m_tasksCondition;
    std::atomic<bool> m_canRun;

private:
    using TTimers = std::map<std::int32_t, std::shared_ptr<TimerInfo>>;
    using TTasks = std::vector<std::unique_ptr<TaskInfo>>;

    TTimers m_timers;
    TTasks m_postedTasks;

private:
    static const std::uint32_t InvalidNearestTimerTime;

public:
    static const std::int32_t INVALID_TIMER_ID = -1;
};

}
}
