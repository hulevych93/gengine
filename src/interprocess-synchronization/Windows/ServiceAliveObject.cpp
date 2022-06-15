#include "ServiceAliveObject.h"

#include <core/Logger.h>
#include <windows.h>

namespace Gengine {
namespace InterprocessSynchronization {
ServiceAliveObject::ServiceAliveObject(const wchar_t* mappingFileName)
    : m_serviceFileMappingHandle(nullptr), m_serviceFileMappingData(nullptr) {
  SECURITY_ATTRIBUTES securityAttributes;
  memset(&securityAttributes, 0, sizeof(SECURITY_ATTRIBUTES));
  securityAttributes.bInheritHandle = true;
  SECURITY_DESCRIPTOR descriptor;
  securityAttributes.lpSecurityDescriptor = &descriptor;
  securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
  ::InitializeSecurityDescriptor(securityAttributes.lpSecurityDescriptor,
                                 SECURITY_DESCRIPTOR_REVISION);
  ::SetSecurityDescriptorDacl(securityAttributes.lpSecurityDescriptor, true,
                              nullptr, false);

  m_serviceFileMappingHandle =
      ::CreateFileMappingW(INVALID_HANDLE_VALUE, &securityAttributes,
                           PAGE_READWRITE, 0, 1024, mappingFileName);
  if (m_serviceFileMappingHandle) {
    m_serviceFileMappingData = ::MapViewOfFile(
        m_serviceFileMappingHandle, FILE_MAP_WRITE, 0, 0, sizeof(DWORD));
    if (m_serviceFileMappingData) {
      auto processId = reinterpret_cast<DWORD*>(m_serviceFileMappingData);
      *processId = ::GetCurrentProcessId();
    }
  } else {
    auto error = ::GetLastError();
    if (error == ERROR_ALREADY_EXISTS) {
      GLOG_ERROR("Failed to create file mapping: %ls; already exists",
                 mappingFileName);
    } else {
      GLOG_ERROR("Failed to create file mapping: %ls; error: %u",
                 mappingFileName, error);
    }
  }
}

ServiceAliveObject::~ServiceAliveObject() {
  GLOG_INFO("Free service alive object.");
  Free();
}

void ServiceAliveObject::Free() {
  if (IsLocked()) {
    if (m_serviceFileMappingHandle) {
      CloseHandle(reinterpret_cast<HANDLE>(m_serviceFileMappingHandle));
      m_serviceFileMappingHandle = nullptr;
    }

    if (m_serviceFileMappingData) {
      UnmapViewOfFile(reinterpret_cast<LPVOID>(m_serviceFileMappingData));
      m_serviceFileMappingData = nullptr;
    }
  }
}

bool ServiceAliveObject::IsLocked() const {
  return m_serviceFileMappingData != nullptr &&
         m_serviceFileMappingHandle != nullptr;
}
}  // namespace InterprocessSynchronization
}  // namespace Gengine