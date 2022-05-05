#include "ServiceUtils.h"

#include <Windows.h>
#include <tchar.h>

#include <ShellAPI.h>

#include <core/Encoding.h>
#include <core/Logger.h>
#include <filesystem/Filesystem.h>

namespace Gengine {
namespace ServiceUtils {

static const std::uint32_t ServiceStopWaitCount(60);

bool AddServiceFailureActions(const std::wstring& name) {
  auto hSCM = ::OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
  if (hSCM) {
    auto hService = ::OpenServiceW(hSCM, name.c_str(), SERVICE_ALL_ACCESS);
    if (hService) {
      SERVICE_FAILURE_ACTIONS servFailActions;
      SC_ACTION failActions[3];
      for (auto index = 0; index < 3; ++index) {
        failActions[index].Type = SC_ACTION_NONE;
        failActions[index].Delay = 10000;
      }

      servFailActions.dwResetPeriod = 86400;
      servFailActions.lpCommand = nullptr;
      servFailActions.lpRebootMsg = nullptr;
      servFailActions.cActions = 3;
      servFailActions.lpsaActions = failActions;

      return ChangeServiceConfig2(hService, SERVICE_CONFIG_FAILURE_ACTIONS,
                                  &servFailActions) == TRUE;
    } else {
      ::CloseServiceHandle(hSCM);
      GLOG_ERROR("Could not open service");
      return false;
    }
  } else {
    GLOG_ERROR("Could not open Service Manager");
    return false;
  }

  return false;
}

bool IsServiceInstalled(const std::wstring& name) {
  auto installed = false;
  auto hSCM = ::OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
  if (hSCM) {
    auto hService = ::OpenServiceW(hSCM, name.c_str(), SERVICE_QUERY_CONFIG);
    if (hService) {
      installed = true;
      ::CloseServiceHandle(hService);
    }
    ::CloseServiceHandle(hSCM);
  }
  return installed;
}

bool IsServiceRunning(const std::wstring& name) {
  auto hSCM = ::OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
  if (hSCM) {
    auto hService = ::OpenServiceW(hSCM, name.c_str(), SERVICE_QUERY_STATUS);
    if (hService) {
      SERVICE_STATUS ssStatus;
      if (QueryServiceStatus(hService, &ssStatus)) {
        if (ssStatus.dwCurrentState == SERVICE_START_PENDING ||
            ssStatus.dwCurrentState == SERVICE_RUNNING ||
            ssStatus.dwCurrentState == SERVICE_STOP_PENDING) {
          return true;
        }
      }
    } else {
      ::CloseServiceHandle(hSCM);
      GLOG_ERROR("Could not open service");
      return false;
    }
  } else {
    GLOG_ERROR("Could not open Service Manager");
    return false;
  }

  return false;
}

bool InstallService(const std::wstring& name) {
  if (!IsServiceInstalled(name)) {
    auto serviceExePath =
        std::wstring{L"\""} + (Filesystem::GetExecutableFilePath()) + L"\"";
    serviceExePath += L" --entry=default";
    auto hSCM = ::OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (hSCM) {
      auto hService = ::CreateServiceW(
          hSCM, name.c_str(), name.c_str(), SERVICE_ALL_ACCESS,
          SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
          SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, serviceExePath.c_str(),
          nullptr, nullptr, L"RPCSS\0", nullptr, nullptr);
      if (hService) {
        AddServiceFailureActions(name);
        ::CloseServiceHandle(hService);
        ::CloseServiceHandle(hSCM);
        return true;
      } else {
        ::CloseServiceHandle(hSCM);
        GLOG_ERROR("Could not create service");
        return false;
      }
    } else {
      GLOG_ERROR("Could not open Service Manager");
      return false;
    }
  } else {
    AddServiceFailureActions(name);
  }

  return true;
}

bool UninstallService(const std::wstring& name) {
  if (IsServiceInstalled(name)) {
    auto hSCM = ::OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (hSCM == nullptr) {
      GLOG_ERROR("Could not open Service Manager");
      return false;
    }

    auto hService = ::OpenServiceW(hSCM, name.c_str(), SERVICE_STOP | DELETE);
    if (hService == nullptr) {
      ::CloseServiceHandle(hSCM);
      GLOG_ERROR("Could not open service");
      return false;
    }

    SERVICE_STATUS ssStatus;
    if (ControlService(hService, SERVICE_CONTROL_STOP, &ssStatus)) {
      Sleep(1000);

      while (QueryServiceStatus(hService, &ssStatus)) {
        if (ssStatus.dwCurrentState == SERVICE_STOP_PENDING) {
          ::Sleep(1000);
        } else {
          break;
        }
      }
    } else {
      auto dwError = GetLastError();
      if (!((dwError == ERROR_SERVICE_NOT_ACTIVE) ||
            (dwError == ERROR_SERVICE_CANNOT_ACCEPT_CTRL &&
             ssStatus.dwCurrentState == SERVICE_STOP_PENDING))) {
        GLOG_ERROR("Can't stop service, error code: %d", dwError);
        GLOG_ERROR("Could not stop service");
      }
    }

    auto bDelete = ::DeleteService(hService) == TRUE;
    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);

    if (bDelete)
      return true;

    GLOG_ERROR("Could not delete service");
    return false;
  }

  return true;
}

bool RunService(const std::wstring& name) {
  if (!IsServiceRunning(name)) {
    auto hSCM = ::OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (hSCM == nullptr) {
      GLOG_ERROR("Could not open Service Manager");
      return false;
    }

    auto hService = ::OpenServiceW(hSCM, name.c_str(), SERVICE_START);
    if (hService == nullptr) {
      ::CloseServiceHandle(hSCM);
      GLOG_ERROR("Could not open service");
      return false;
    }

    SERVICE_STATUS ssStatus;
    while (QueryServiceStatus(hService, &ssStatus)) {
      if (ssStatus.dwCurrentState == SERVICE_STOP_PENDING) {
        Sleep(1000);
        return true;
      } else {
        break;
      }
    }

    if (::StartService(hService, 0, NULL)) {
      ::CloseServiceHandle(hService);
      ::CloseServiceHandle(hSCM);
      return true;
    }

    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);
    return false;
  }

  return true;
}

bool StopService(const std::wstring& name) {
  if (IsServiceRunning(name)) {
    auto hSCM = ::OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (hSCM != nullptr) {
      auto hService = ::OpenServiceW(hSCM, name.c_str(),
                                     SERVICE_STOP | SERVICE_QUERY_STATUS);
      if (hService != nullptr) {
        SERVICE_STATUS ssStatus;
        if (ControlService(hService, SERVICE_CONTROL_STOP, &ssStatus) == TRUE ||
            (GetLastError() == ERROR_SERVICE_CANNOT_ACCEPT_CTRL &&
             ssStatus.dwCurrentState == SERVICE_STOP_PENDING)) {
          Sleep(1000);

          std::uint32_t waitCount{0};
          while (QueryServiceStatus(hService, &ssStatus)) {
            if (ssStatus.dwCurrentState == SERVICE_STOP_PENDING) {
              Sleep(1000);
              ++waitCount;
              if (waitCount >= ServiceStopWaitCount) {
                ::CloseServiceHandle(hService);
                ::CloseServiceHandle(hSCM);
                return TerminateService(name) || !IsServiceRunning(name);
              }
            } else if (ssStatus.dwCurrentState == SERVICE_STOPPED) {
              break;
            }
          }

          Sleep(1000);

          ::CloseServiceHandle(hService);
          ::CloseServiceHandle(hSCM);

          return true;
        } else {
          auto error = GetLastError();
          if (error != ERROR_SERVICE_NOT_ACTIVE) {
            GLOG_WARNING("Can't stop service, error code: %d", error);
            GLOG_ERROR("Could not stop service");
          }

          ::CloseServiceHandle(hService);
          ::CloseServiceHandle(hSCM);

          return false;
        }
      } else {
        ::CloseServiceHandle(hSCM);
        GLOG_ERROR("Could not open service");
      }
    } else {
      GLOG_ERROR("Could not open Service Manager");
    }
  }

  return true;
}

bool StopRequest(const std::wstring& name) {
  if (IsServiceRunning(name)) {
    auto hSCM = ::OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (hSCM != nullptr) {
      auto hService = ::OpenServiceW(hSCM, name.c_str(),
                                     SERVICE_STOP | SERVICE_QUERY_STATUS);
      if (hService != nullptr) {
        SERVICE_STATUS ssStatus;
        if (ControlService(hService, SERVICE_CONTROL_STOP, &ssStatus) == TRUE ||
            (GetLastError() == ERROR_SERVICE_CANNOT_ACCEPT_CTRL &&
             ssStatus.dwCurrentState == SERVICE_STOP_PENDING)) {
          Sleep(1000);

          ::CloseServiceHandle(hService);
          ::CloseServiceHandle(hSCM);

          return true;
        } else {
          auto error = GetLastError();
          if (error != ERROR_SERVICE_NOT_ACTIVE) {
            GLOG_WARNING("Can't stop service, error code: %d", error);
            GLOG_ERROR("Could not stop service");
          }

          ::CloseServiceHandle(hService);
          ::CloseServiceHandle(hSCM);

          return false;
        }
      } else {
        ::CloseServiceHandle(hSCM);
        GLOG_ERROR("Could not open service");
      }
    } else {
      GLOG_ERROR("Could not open Service Manager");
    }
  }

  return true;
}

bool TerminateService(const std::wstring& name) {
  if (IsServiceRunning(name)) {
    auto serviceExePath = GetServiceExePath(name);
    if (!serviceExePath.empty()) {
      // return ProcessHelper::KillProcess(serviceExePath);
    }
  }

  return false;
}

std::wstring EraseQuotesFromPath(const std::wstring& path) {
  std::wstring resultPath(path);
  if (resultPath[0] == L'\"') {
    resultPath.erase(0, 1);
  }
  if (resultPath.back() == L'\"') {
    resultPath.pop_back();
  }
  return resultPath;
}

std::wstring GetServiceExePath(const std::wstring& name) {
  auto scm = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
  if (scm != nullptr) {
    auto service = OpenServiceW(scm, name.c_str(), SERVICE_QUERY_CONFIG);
    if (service != nullptr) {
      LPQUERY_SERVICE_CONFIG serviceConfig;
      DWORD bytesNeeded = 0, bufSize = 0;
      QueryServiceConfig(service, nullptr, 0, &bytesNeeded);

      if (ERROR_INSUFFICIENT_BUFFER == GetLastError()) {
        bufSize = bytesNeeded;
        serviceConfig = reinterpret_cast<LPQUERY_SERVICE_CONFIG>(
            ::LocalAlloc(LMEM_FIXED, bufSize));
        if (QueryServiceConfig(service, serviceConfig, bufSize, &bytesNeeded) ==
            TRUE) {
          auto serviceExePath =
              EraseQuotesFromPath(serviceConfig->lpBinaryPathName);
          CloseServiceHandle(scm);
          CloseServiceHandle(service);
          LocalFree(serviceConfig);

          return serviceExePath;
        } else {
          GLOG_ERROR("QueryServiceConfig failed: [%d]", GetLastError());
          CloseServiceHandle(scm);
          CloseServiceHandle(service);
        }
      } else {
        GLOG_ERROR("QueryServiceConfig failed: [%d]", GetLastError());
        CloseServiceHandle(scm);
        CloseServiceHandle(service);
      }
    } else {
      GLOG_ERROR("OpenService failed: [%d]", GetLastError());
      CloseServiceHandle(scm);
    }
  } else {
    GLOG_ERROR("OpenSCManager failed: [%d]", GetLastError());
  }

  return std::wstring();
}
}  // namespace ServiceUtils
}  // namespace Gengine