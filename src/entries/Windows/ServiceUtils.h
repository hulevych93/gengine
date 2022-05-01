#pragma once

#include <string>
#include <cstdint>

namespace Gengine {
namespace ServiceUtils {
bool AddServiceFailureActions(const std::wstring& name);
bool IsServiceInstalled(const std::wstring& name);
bool IsServiceRunning(const std::wstring& name);
bool InstallService(const std::wstring& name);
bool UninstallService(const std::wstring& name);
bool RunService(const std::wstring& name);
bool StopService(const std::wstring& name);
bool StopRequest(const std::wstring& name);
bool TerminateService(const std::wstring& name);
std::wstring GetServiceExePath(const std::wstring& name);
}
}