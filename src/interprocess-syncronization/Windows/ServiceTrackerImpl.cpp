#include "ServiceTrackerImpl.h"

#include <core/Logger.h>
#include <multithreading/ThreadUtils.h>
#include <windows.h>

namespace Gengine {
namespace InterprocessSynchronization {
ServiceTrackerImpl::ServiceTrackerImpl(
    const std::wstring& mappingFileName,
    ServiceTracker::terminate_handler handler)
    : ServiceTracker(handler),
      m_mappingFileName(mappingFileName),
      m_fileMappingHandle(nullptr),
      m_fileMappingData(nullptr) {
  m_stopEvent.Create(true, false);
}

ServiceTrackerImpl::~ServiceTrackerImpl() {
  if (m_thread) {
    m_thread->join();
    UnmapViewOfFile(reinterpret_cast<LPVOID>(m_fileMappingData));
    CloseHandle(reinterpret_cast<HANDLE>(m_fileMappingHandle));
  }
}

void ServiceTrackerImpl::StartInternal() {
  m_fileMappingData =
      MapViewOfFile(m_fileMappingHandle, FILE_MAP_READ, 0, 0, sizeof(DWORD));
  m_thread = std::make_unique<std::thread>(
      &ServiceTrackerImpl::TrackerThreadProc, this);
  Multithreading::ThreadUtils::SetThreadName(*m_thread,
                                             L"ServiceTrackerThread");
}

void ServiceTrackerImpl::StopInternal() {
  m_stopEvent.Set();
}

bool ServiceTrackerImpl::IsCanStart() {
  m_fileMappingHandle =
      OpenFileMappingW(FILE_MAP_READ, FALSE, m_mappingFileName.c_str());
  return m_fileMappingHandle != nullptr;
}

void ServiceTrackerImpl::TrackerThreadProc() {
  DWORD processId = *(reinterpret_cast<DWORD*>(m_fileMappingData));
  GLOG_INFO("Tracking process %d", processId);
  HANDLE serviceProcess = ::OpenProcess(SYNCHRONIZE, FALSE, processId);

  std::vector<HANDLE> changeHandles(2);
  changeHandles[0] = m_stopEvent.GetOSHandle();
  changeHandles[1] = serviceProcess;

  auto waitStatus = ::WaitForMultipleObjects(
      changeHandles.size(), &changeHandles[0], FALSE, INFINITE);
  switch (waitStatus) {
    case WAIT_OBJECT_0: {
      GLOG_ERROR("Tracker stopped.");
      break;
    } break;
    case (WAIT_OBJECT_0 + 1): {
      GLOG_INFO("Service died. Terminating processes...");
      Terminate();
    } break;
    case WAIT_FAILED:
      GLOG_ERROR("Wait failed! Error: %d", ::GetLastError());
      break;
    case WAIT_TIMEOUT:
      GLOG_ERROR("Timeout in infinite mode? Is it possible?");
      break;
    default:
      GLOG_ERROR("unknown wait status: %u", waitStatus);
      break;
  }
}
}  // namespace InterprocessSynchronization
}  // namespace Gengine