#pragma once

#include <interprocess-syncronization/ServiceTracker.h>
#include <atomic>
#include <string>
#include <thread>

namespace Gengine {
namespace InterprocessSynchronization {
class LinuxDaemonTracker : public ServiceTracker {
 public:
  LinuxDaemonTracker(const std::string& fileName, terminate_handler handler);
  ~LinuxDaemonTracker();

 protected:
  void StartInternal() override;
  void StopInternal() override;
  bool IsCanStart() override;

 private:
  std::thread m_thread;
  std::atomic<bool> m_canRun;
  void TrackerThreadProc();

 private:
  std::string m_mappingFileName;
  int m_file;
};
}  // namespace InterprocessSynchronization
}  // namespace Gengine
