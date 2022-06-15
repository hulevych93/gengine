#pragma once

#include <interprocess-synchronization/ServiceTracker.h>
#include <atomic>
#include <string>
#include <thread>

namespace Gengine {
namespace InterprocessSynchronization {
class LinuxDaemonTracker final : public ServiceTracker {
 public:
  LinuxDaemonTracker(const std::string& fileName, terminate_handler handler);
  LinuxDaemonTracker(const LinuxDaemonTracker&&) = delete;
  LinuxDaemonTracker(LinuxDaemonTracker&&) = delete;
  ~LinuxDaemonTracker();

 protected:
  void StartInternal() override;
  void StopInternal() override;
  bool IsCanStart() override;

 private:
  std::thread m_thread;
  void RunLoop();

 private:
  std::string m_mappingFileName;
  int m_selfSignalPipe[2];
  int m_aliveFile;
};
}  // namespace InterprocessSynchronization
}  // namespace Gengine
