#include <entries/TestEntry.h>
#include <gtest/gtest.h>

#include <gengine/gengine.h>

#include <interprocess-synchronization/Unix/DaemonAliveObject.h>
#include <interprocess-synchronization/Unix/LinuxDaemonTracker.h>
#include <interprocess-synchronization/Unix/LinuxSingleInstanceRegistrator.h>

using namespace Gengine;
using namespace Entries;
using namespace InterprocessSynchronization;

namespace {
class SyncronizationTest : public testing::Test {};

constexpr const wchar_t* LockFileNameW = L"/tmp/some_random_file_2";
constexpr const wchar_t* FileNameW = L"/tmp/some_random_file";
constexpr const char* FileName = "/tmp/some_random_file";

struct TaskStartedEnsurer final {
  TaskStartedEnsurer() { m_future = m_trigger.get_future(); }
  ~TaskStartedEnsurer() { m_future.wait(); }

  void IAmStarted() { m_trigger.set_value(); }

 private:
  std::promise<void> m_trigger;
  std::future<void> m_future;
};

}  // namespace

TEST_F(SyncronizationTest, daemonAliveObject) {
  DaemonAliveObject object(FileNameW);
  EXPECT_TRUE(object.IsLocked());
}

TEST_F(SyncronizationTest, daemonTracker) {
  DaemonAliveObject object(FileNameW);
  EXPECT_TRUE(object.IsLocked());

  bool terminated = false;
  {
    TaskStartedEnsurer ensurer;

    LinuxDaemonTracker tracker{FileName, [&]() {
                                 terminated = true;
                                 ensurer.IAmStarted();
                               }};
    EXPECT_TRUE(tracker.Start());
    object.Free();
  }

  EXPECT_TRUE(terminated);
}

TEST_F(SyncronizationTest, singleInstanceRegistrator) {
  LinuxSingleInstanceRegistrator singleInstance{LockFileNameW};
  EXPECT_TRUE(singleInstance.RegisterInstance());
  singleInstance.UnregisterInstance();
}

REGISTER_TESTS_ENTRY(GTestModule)
IMPLEMENT_CONSOLE_ENTRY
