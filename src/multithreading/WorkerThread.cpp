#include <multithreading/ThreadUtils.h>
#include <multithreading/WorkerThread.h>
#include <functional>
#include <sstream>

#include <boost/optional.hpp>
#include <chrono>

namespace Gengine {
namespace Multithreading {
using namespace Services;

class WorkerThread::Future final : public Services::IFuture {
 public:
  Future() : m_completed(ManualResetTag{}) {}

  void Wait(const std::chrono::system_clock::duration timeout) {
    m_completed.Wait(timeout);
  }
  void Complete() { m_completed.Set(); }
  void Reset() { m_completed.Reset(); }

 private:
  Event m_completed;
};

struct WorkerThread::FutureLock final {
  FutureLock(Future& future) : m_future(future) { m_future.Reset(); }

  ~FutureLock() { m_future.Complete(); }

 private:
  Future& m_future;
};

WorkerThread::WorkerThread(const std::string& name)
    : m_canRun(true),
      m_name(std::make_pair(name, false)),
      m_executorThread([&]() { RunLoop(); }) {}

WorkerThread::~WorkerThread() {
  Dispose(std::chrono::seconds{0});
}

void WorkerThread::RunLoop() {
  while (true) {
    if (!m_canRun.load()) {
      std::lock_guard<std::mutex> locker(m_mutex);

      if (std::chrono::system_clock::now() > m_disposeTime) {
        break;
      }

      if (m_postedTasks.empty()) {
        break;
      }
    }

    if (!m_name.second)
      m_name.second = SetThreadName(m_executorThread, m_name.first);

    TasksType tasks;

    {
      std::lock_guard<std::mutex> locker(m_mutex);
      tasks = std::move(m_postedTasks);
    }

    for (const auto& task : tasks) {
      task.task();
      if (auto future = task.future.lock()) {
        future->Complete();
      }
    }

    auto now = std::chrono::system_clock::now();
    boost::optional<std::chrono::system_clock::time_point> nearestTimerTime;

    TimersType localTimers;

    {
      std::lock_guard<std::mutex> locker(m_mutex);
      localTimers = m_timers;
    }

    for (auto& localTimerInfo : localTimers) {
      if (localTimerInfo->scheduledTime <= now) {
        FutureLock futureLock{*localTimerInfo->future};

        localTimerInfo->task();
        now = std::chrono::system_clock::now();
        localTimerInfo->scheduledTime = now + localTimerInfo->interval;
      }

      if (!nearestTimerTime.has_value() || localTimerInfo->scheduledTime < nearestTimerTime) {
        nearestTimerTime = localTimerInfo->scheduledTime;
      }
    }

    if (m_canRun.load()) {
      now = std::chrono::system_clock::now();
      if (nearestTimerTime.has_value()) {
        if (nearestTimerTime > now) {
          m_tasksCondition.Wait(nearestTimerTime.get() - now);
        }
      } else {
        m_tasksCondition.Wait(WaitInfinite);
      }
    }
  }
}

std::string WorkerThread::GetID() const {
  auto id = m_executorThread.get_id();
  std::stringstream str;
  str << id;
  return str.str();
}

void WorkerThread::Dispose(const std::chrono::system_clock::duration timeout) {
  if (m_executorThread.joinable()) {
    m_disposeTime = std::chrono::system_clock::now() + timeout;
    m_canRun.store(false);

    if (std::this_thread::get_id() != m_executorThread.get_id()) {
      m_tasksCondition.Set();
      m_executorThread.join();
    }

    for (const auto& task : m_postedTasks) {
      if (auto future = task.future.lock())
        future->Complete();
    }
    m_postedTasks.clear();

    for (const auto& timer : m_timers) {
      timer->future->Complete();
    }
    m_timers.clear();
  }
}

std::shared_ptr<IFuture> WorkerThread::PostTask(task_t task) {
  if (!m_canRun.load()) {
    return std::shared_ptr<IFuture>();
  }

  auto future = std::make_shared<Future>();
  TaskInfo info{std::move(task), future};

  {
    std::lock_guard<std::mutex> locker(m_mutex);
    m_postedTasks.emplace_back(std::move(info));
  }

  m_tasksCondition.Set();
  return future;
}

void WorkerThread::PostTaskAndWait(task_t task) {
  if (m_executorThread.get_id() == std::this_thread::get_id()) {
    task();
  }

  auto future = PostTask(std::move(task));
  if (future) {
    future->Wait(WaitInfinite);
  }
}

std::shared_ptr<IFuture> WorkerThread::PostTask(cancelable_task_t task) {
  return PostTask([this, task = std::move(task)]() {
    task([this]() { return !m_canRun.load(); });
  });
}

void WorkerThread::PostTaskAndWait(cancelable_task_t task) {
  PostTaskAndWait([this, task = std::move(task)]() {
    task([this]() { return !m_canRun.load(); });
  });
}

std::int32_t WorkerThread::StartTimer(
    task_t task,
    std::chrono::system_clock::duration interval,
    std::chrono::system_clock::duration firstDelay) {
  if (!m_canRun.load()) {
    return -1;
  }

  std::int32_t newId = 0;

  if (!m_timers.empty()) {
    newId = m_timers.back()->id + 1;
  }

  auto info = std::make_shared<TimerInfo>();
  info->interval = interval;

  info->scheduledTime = std::chrono::system_clock::now() + firstDelay;
  info->task = std::move(task);
  info->id = newId;

  {
    auto future = std::make_shared<Future>();
    future->Complete();
    info->future = std::move(future);
  }

  {
    std::lock_guard<std::mutex> locker(m_mutex);
    m_timers.emplace_back(std::move(info));
  }

  m_tasksCondition.Set();
  return newId;
}

std::shared_ptr<IFuture> WorkerThread::StopTimer(const std::int32_t timerId) {
  std::lock_guard<std::mutex> locker(m_mutex);
  auto index = std::min<std::uint32_t>(timerId, m_timers.size() - 1);
  const auto beginIter = m_timers.rend() - (index + 1);
  auto it =
      std::find_if(beginIter, m_timers.rend(),
                   [timerId](const auto& info) { return info->id == timerId; });
  if (it != m_timers.rend()) {
    auto future = (*it)->future;
    m_timers.erase(std::next(it).base());
    return future;
  }
  return std::shared_ptr<Future>();
}

}  // namespace Multithreading
}  // namespace Gengine
