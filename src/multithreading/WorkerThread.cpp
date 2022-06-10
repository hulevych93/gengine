#include <multithreading/Future.h>
#include <multithreading/ThreadUtils.h>
#include <multithreading/WorkerThread.h>
#include <functional>
#include <sstream>

#include <chrono>

namespace Gengine {
namespace Multithreading {
using namespace Services;

std::uint32_t getTimeInMills() {
  const auto now = std::chrono::system_clock::now();
  const auto duration = now.time_since_epoch();
  return std::chrono::duration_cast<std::chrono::milliseconds>(duration)
      .count();
}

const std::uint32_t WorkerThread::InvalidNearestTimerTime(-1);

WorkerThread::WorkerThread(const std::wstring& name, bool canRun)
    : m_canRun(canRun), m_name(std::make_pair(name, false)) {
  if (m_canRun) {
    Initialize();
  }
}

void WorkerThread::Initialize() {
  if (!m_executorThread) {
    m_executorThread = std::make_unique<std::thread>(
        std::bind(&WorkerThread::ThreadProc, this));
    m_name.second = ThreadUtils::SetThreadName(*m_executorThread, m_name.first);
  }
}

WorkerThread::~WorkerThread() {
  Dispose();
}

std::string WorkerThread::GetID() const {
  auto id = m_executorThread->get_id();
  std::stringstream str;
  str << id;
  return str.str();
}

void WorkerThread::PostDeinializationTask(task_t task) {
  auto handler = [this, task]() {
    Dispose();
    task();
  };
  PostTaskAndWait(handler);
}

void WorkerThread::Dispose() {
  if (m_canRun) {
    m_canRun = false;
  }
  if (m_executorThread->joinable() &&
      std::this_thread::get_id() != m_executorThread->get_id()) {
    m_tasksCondition.Set();
    m_executorThread->join();
  }
  ClearTasks();
}

std::shared_ptr<IFuture> WorkerThread::PostTask(task_t task) {
  if (!m_canRun) {
    return std::shared_ptr<IFuture>();
  }

  auto result = std::make_shared<Future>(true);
  {
    auto info = std::make_unique<TaskInfo>();
    info->asyncResult = result;
    info->task = std::move(task);

    std::lock_guard<std::mutex> locker(m_tasksMutex);
    m_postedTasks.emplace_back(std::move(info));
  }

  m_tasksCondition.Set();
  return result;
}

void WorkerThread::PostTaskAndWait(task_t task) {
  if (m_executorThread->get_id() == std::this_thread::get_id()) {
    task();
  } else {
    if (!m_canRun) {
      return;
    }

    auto pResult = PostTask(std::move(task));
    if (pResult) {
      pResult->Wait(Event::WaitInfinite);
    }
  }
}

std::int32_t WorkerThread::StartTimer(task_t task,
                                      std::uint32_t intervalMS,
                                      std::uint32_t firstDelayMS /*= 0*/) {
  if (!m_canRun) {
    return -1;
  }

  std::int32_t newId = 0;
  {
    if (!m_timers.empty()) {
      newId = m_timers.rbegin()->first + 1;
    }

    auto info = std::make_unique<TimerInfo>();
    info->intervalMS = intervalMS;
    info->nextScheduledTime = getTimeInMills() + firstDelayMS;
    info->task = std::move(task);
    info->asyncResult = std::make_shared<Future>(false);

    std::lock_guard<std::mutex> locker(m_tasksMutex);
    m_timers[newId] = std::move(info);
  }

  m_tasksCondition.Set();
  return newId;
}

std::shared_ptr<IFuture> WorkerThread::StopTimer(std::int32_t timerId) {
  std::lock_guard<std::mutex> locker(m_tasksMutex);
  auto it = m_timers.find(timerId);
  if (it != m_timers.end()) {
    auto result = it->second->asyncResult;
    m_timers.erase(it);
    return result;
  }

  return std::shared_ptr<Future>();
}

void WorkerThread::StopAndWaitTimer(std::int32_t timerId) {
  auto result = StopTimer(timerId);
  if (result) {
    result->Wait(Event::WaitInfinite);
  }
}

void WorkerThread::ThreadProc() {
  while (m_canRun) {
    TTasks tasks;

    {
      std::lock_guard<std::mutex> locker(m_tasksMutex);
      tasks = std::move(m_postedTasks);
    }

    for (const auto& task : tasks) {
      task->task();
      task->asyncResult->Complete();
    }

    auto now = getTimeInMills();
    decltype(now) nearestTimerTime = InvalidNearestTimerTime;

    TTimers localTimers;

    {
      std::lock_guard<std::mutex> locker(m_tasksMutex);
      localTimers = m_timers;
    }

    for (const auto& localTimer : localTimers) {
      if (localTimer.second->nextScheduledTime <= now) {
        localTimer.second->asyncResult->Reset();
        localTimer.second->task();
        now = getTimeInMills();
        localTimer.second->nextScheduledTime =
            now + localTimer.second->intervalMS;
        localTimer.second->asyncResult->Complete();
      }

      if (localTimer.second->nextScheduledTime < nearestTimerTime) {
        nearestTimerTime = localTimer.second->nextScheduledTime;
      }
    }

    now = getTimeInMills();
    if (nearestTimerTime > now) {
      m_tasksCondition.Wait(nearestTimerTime - now);
    }

    if (!m_name.second && m_executorThread)
      m_name.second =
          ThreadUtils::SetThreadName(*m_executorThread, m_name.first);
  }
}

void WorkerThread::ClearTasks() {
  std::lock_guard<std::mutex> locker(m_tasksMutex);
  for (const auto& task : m_postedTasks) {
    task->asyncResult->Cancel();
  }
  m_postedTasks.clear();
  for (const auto& timer : m_timers) {
    timer.second->asyncResult->Cancel();
  }
  m_timers.clear();
}
}  // namespace Multithreading
}  // namespace Gengine
