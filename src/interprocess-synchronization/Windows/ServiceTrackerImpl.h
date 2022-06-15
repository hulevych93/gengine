#pragma once

#include <interprocess-synchronization/ServiceTracker.h>
#include <thread>

namespace Gengine {
namespace InterprocessSynchronization {
class ServiceTrackerImpl : public ServiceTracker {
 public:
  ServiceTrackerImpl(const std::wstring& mappingFileName,
                     ServiceTracker::terminate_handler handler);
  ServiceTrackerImpl(const ServiceTrackerImpl&) = delete;
  ServiceTrackerImpl(ServiceTrackerImpl&&) = delete;
  virtual ~ServiceTrackerImpl();

 protected:
  void StartInternal() override;
  void StopInternal() override;
  bool IsCanStart() override;

 private:
  std::unique_ptr<std::thread> m_thread;
  void TrackerThreadProc();

 private:
  std::wstring m_mappingFileName;
  void* m_fileMappingHandle;
  void* m_fileMappingData;
  void* m_stopEvent;
};
}  // namespace InterprocessSynchronization
}  // namespace Gengine