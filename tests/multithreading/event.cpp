#include <gtest/gtest.h>

#include <entries/TestEntry.h>
#include <gengine/gengine.h>

class MultithreadingTest : public testing::Test {
 protected:
  void SetUp() {}

  void TearDown() {}
};

#include <core/Logger.h>
#include <multithreading/Event.h>

#include <chrono>
#include <thread>

using namespace Gengine;
using namespace Entries;
using namespace Multithreading;

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

TEST_F(MultithreadingTest, eventWaitTimeout) {
  Event event;

  waitTimeTest([&]() { EXPECT_FALSE(event.Wait(100)); }, 100, 110);
}

TEST_F(MultithreadingTest, eventWait) {
  Event event;

  FireThread fireThread(event, 100);

  waitTimeTest([&]() { EXPECT_TRUE(event.Wait(Event::WaitInfinite)); }, 100,
               110);
}

TEST_F(MultithreadingTest, eventWaitManualReset) {
  Event event{ManualResetTag{}};

  FireThread fireThread(event, 100);

  waitTimeTest([&]() { EXPECT_TRUE(event.Wait(Event::WaitInfinite)); }, 100,
               110);

  waitTimeTest([&]() { EXPECT_TRUE(event.Wait(Event::WaitInfinite)); }, 0, 10);

  event.Reset();

  FireThread fireThread2(event, 100);

  waitTimeTest([&]() { EXPECT_TRUE(event.Wait(Event::WaitInfinite)); }, 100,
               110);
}

TEST_F(MultithreadingTest, eventWaitAutoReset) {
  Event event;

  FireThread fireThread(event, 100);

  waitTimeTest([&]() { EXPECT_TRUE(event.Wait(Event::WaitInfinite)); }, 100,
               110);

  FireThread fireThread2(event, 100);

  waitTimeTest([&]() { EXPECT_TRUE(event.Wait(Event::WaitInfinite)); }, 100,
               110);
}

REGISTER_TESTS_ENTRY(GTestModule)
IMPLEMENT_CONSOLE_ENTRY
