#include <gtest/gtest.h>

#include <entries/TestEntry.h>
#include <gengine/gengine.h>

#include <core/Logger.h>
#include <multithreading/WorkerThread.h>

#include <chrono>
#include <thread>

using namespace Gengine;
using namespace Entries;
using namespace Multithreading;

namespace {

struct TimeMeassurement final {
  TimeMeassurement() { m_start = std::chrono::system_clock::now(); }

  std::uint32_t getElapsedMills() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now() - m_start)
        .count();
  }

  std::chrono::system_clock::time_point m_start;
};

struct FireThread final {
  FireThread(Event& event, const std::uint32_t timeout)
      : m_event(event), m_thr([&]() {
          std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
          event.Set();
        }) {}
  ~FireThread() { m_thr.join(); }

 private:
  Event& m_event;
  std::thread m_thr;
};

}  // namespace

namespace UnitTests {

class WorkerThreadTest : public testing::Test {
 protected:
  WorkerThreadTest() : worker("worker") {}

  size_t getTasksCount() const { return worker.m_postedTasks.size(); }
  size_t getTimersCount() const { return worker.m_timers.size(); }

  WorkerThread worker;
};

template <typename Func>
void waitTimeTest(Func&& function,
                  std::uint32_t lowBound,
                  std::uint32_t uppBound) {
  TimeMeassurement timePoint;
  function();
  const auto elapsedMills = timePoint.getElapsedMills();
  EXPECT_LE(lowBound, elapsedMills);
  EXPECT_GE(uppBound, elapsedMills);
}

TEST_F(WorkerThreadTest, postTaskAndWait) {
  std::uint32_t value = 0u;
  auto future = worker.PostTask([&]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    value = 354u;
  });

  future->Wait(Event::WaitInfinite);
  EXPECT_EQ(354u, value);

  EXPECT_EQ(0u, getTasksCount());
}

TEST_F(WorkerThreadTest, postCancelableTask) {
  auto future = worker.PostTask([&](Services::calcelled_callback is_cancelled) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (is_cancelled())
      return;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  });

  waitTimeTest([&]() { future->Wait(std::chrono::hours{24}); }, 90, 200);
}

TEST_F(WorkerThreadTest, stopTimer) {
  std::uint32_t value = 0u;
  const auto id =
      worker.StartTimer([&]() { ++value; }, std::chrono::milliseconds{10});

  auto future = worker.StopTimer(id);
  if (future)
    future->Wait(std::chrono::hours{24});

  const auto oldValue = value;
  std::this_thread::sleep_for(std::chrono::milliseconds(30));

  EXPECT_EQ(oldValue, value);
}

TEST_F(WorkerThreadTest, waitTimer) {
  std::promise<void> trigger;
  auto waiter = trigger.get_future();

  const auto id = worker.StartTimer(
      [&]() {
        trigger.set_value();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      },
      std::chrono::milliseconds{10});

  waiter.wait();
  waitTimeTest(
      [&]() {
        auto future = worker.StopTimer(id);
        if (future)
          future->Wait(std::chrono::hours{24});
      },
      90, 110);
}

TEST_F(WorkerThreadTest, multipleTasksOrder) {
  std::vector<std::int32_t> actual(1000, 0);
  std::vector<std::int32_t> expected(1000, 0);

  for (int i = 0; i < 1000u; ++i) {
    worker.PostTask([i, &actual]() { actual[i] = i; });
    expected[i] = i;
  }

  worker.Dispose(Event::WaitInfinite);

  EXPECT_EQ(expected, actual);
}

TEST_F(WorkerThreadTest, multipleTimers) {
  std::vector<std::int32_t> ids(1000, 0);

  for (int i = 0; i < 1000; ++i) {
    ids[ids.size() - (i + 1)] =
        worker.StartTimer([]() {}, std::chrono::milliseconds{10});
  }

  for (const auto& id : ids) {
    auto future = worker.StopTimer(id);
    if (future)
      future->Wait(std::chrono::hours{24});
  }

  EXPECT_EQ(0u, getTimersCount());
}

TEST_F(WorkerThreadTest, stopTimerReverseCheck) {
  auto id1 = worker.StartTimer([]() {}, std::chrono::milliseconds{10});
  auto id2 = worker.StartTimer([]() {}, std::chrono::milliseconds{10});

  worker.StopTimer(id1)->Wait(std::chrono::hours{24});
  worker.StopTimer(id2)->Wait(std::chrono::hours{24});

  EXPECT_EQ(0u, getTimersCount());
}

}  // namespace UnitTests
