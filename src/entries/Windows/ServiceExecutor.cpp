#include "ServiceExecutor.h"

#include <ShellAPI.h>
#include <Windows.h>
#include <tchar.h>
#include <thread>

#include <api/entries/IEntry.h>
#include <entries/Windows/ServiceUtils.h>

#include <core/Encoding.h>
#include <core/Logger.h>

namespace Gengine {
namespace Entries {

ServiceExecutor::StatusContext::StatusContext()
    : handle(nullptr), status(std::make_unique<SERVICE_STATUS>()) {
  status->dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  status->dwCurrentState = SERVICE_STOPPED;
  status->dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP |
                               SERVICE_ACCEPT_POWEREVENT |
                               SERVICE_ACCEPT_HARDWAREPROFILECHANGE;

  OSVERSIONINFO ver;
  memset(&ver, 0, sizeof(ver));
  ver.dwOSVersionInfoSize = sizeof(ver);
  GetVersionEx(&ver);
  if (ver.dwMajorVersion >= 5 && ver.dwMinorVersion >= 1) {
    status->dwControlsAccepted |= SERVICE_ACCEPT_SESSIONCHANGE;
  }

  status->dwWin32ExitCode = 0;
  status->dwServiceSpecificExitCode = 0;
  status->dwCheckPoint = 0;
  status->dwWaitHint = 0;
}

void ServiceExecutor::StatusContext::Update(std::uint32_t code) {
  assert(handle != nullptr);
  if (handle) {
    status->dwCurrentState = code;
    ::SetServiceStatus(reinterpret_cast<SERVICE_STATUS_HANDLE>(handle),
                       status.get());
  }
}

ServiceExecutor* ServiceExecutor::m_instance = nullptr;

ServiceExecutor::ServiceExecutor(IEntry& entry)
    : SimpleExecutor(entry), m_controlListener("AAAA") {
  m_instance = this;
  m_stopEvent.Create(true, false);
}

bool ServiceExecutor::Execute(void* args) {
  std::wstring name;
  GetEntry().GetAppName(&name);
  SERVICE_TABLE_ENTRYW st[] = {
      {const_cast<wchar_t*>(name.data()), _ServiceMain}, {nullptr, nullptr}};
  if (::StartServiceCtrlDispatcherW(st) == 0) {
    m_status.status->dwWin32ExitCode = GetLastError();
    GLOG_ERROR("Exit code: %d", m_status.status->dwWin32ExitCode);
  }
  return true;
}

bool ServiceExecutor::CreateProcessors(
    std::vector<std::unique_ptr<ICmdProcessor>>* processors) {
  assert(processors);
  processors->emplace_back(std::make_unique<SWindowsServiceInstallCommand>());
  processors->emplace_back(std::make_unique<SWindowsServiceUninstallCommand>());
  processors->emplace_back(std::make_unique<SWindowsServiceStartCommand>());
  processors->emplace_back(std::make_unique<SWindowsServiceStopCommand>());
  processors->emplace_back(std::make_unique<SWindowsServiceDebugCommand>());
  return true;
}

void __stdcall ServiceExecutor::_ServiceMain(unsigned long dwArgc,
                                             wchar_t* lpszArgv[]) {
  m_instance->ServiceMain(dwArgc, lpszArgv);
}

unsigned long __stdcall ServiceExecutor::_HandlerEx(unsigned long dwControl,
                                                    unsigned long dwEventType,
                                                    void* lpEventData,
                                                    void* lpContext) {
  return m_instance->HandlerEx(dwControl, dwEventType, lpEventData, lpContext);
}

void ServiceExecutor::ServiceMain(std::uint32_t argc, wchar_t** argv) {
  std::wstring name;
  GetEntry().GetAppName(&name);
  m_status.handle = RegisterServiceCtrlHandlerExW(
      const_cast<wchar_t*>(name.data()), _HandlerEx, nullptr);
  if (m_status.handle) {
    GLOG_DEBUG("Registered service handlerEx");

    m_status.Update(SERVICE_START_PENDING);
    m_status.status->dwWin32ExitCode = ERROR_SUCCESS;
    m_status.status->dwCheckPoint = 0;
    m_status.status->dwWaitHint = 0;

    GetEntry().Initialize();
    GetEntry().Execute(nullptr);
    m_status.Update(SERVICE_RUNNING);
    GLOG_DEBUG("Executed");
    m_stopEvent.Wait(-1);

    std::int32_t code;
    GetEntry().Exit(&code);
    GetEntry().Finalize();
    GLOG_DEBUG("Stopped: %d", code);
    m_status.Update(SERVICE_STOPPED);
  }
}

std::uint32_t ServiceExecutor::HandlerEx(std::uint32_t control,
                                         std::uint32_t eventType,
                                         void* eventData,
                                         void* context) {
  switch (control) {
    case SERVICE_CONTROL_STOP:
      OnStop();
      break;
    case SERVICE_CONTROL_PAUSE:
      OnPause();
      break;
    case SERVICE_CONTROL_CONTINUE:
      OnContinue();
      break;
    case SERVICE_CONTROL_INTERROGATE:
      OnInterrogate();
      break;
    case SERVICE_CONTROL_SHUTDOWN:
      OnShutdown();
      break;
    case SERVICE_CONTROL_HARDWAREPROFILECHANGE:
      OnHardwareProfileChange();
      break;
    case SERVICE_CONTROL_POWEREVENT:
      OnPowerEvent();
      break;
    case SERVICE_CONTROL_SESSIONCHANGE:
      OnSessionChange(eventType,
                      reinterpret_cast<WTSSESSION_NOTIFICATION*>(eventData));
      break;
  }

  return NO_ERROR;
}

void ServiceExecutor::OnStop() {
  GLOG_INFO("Stop");
  m_status.Update(SERVICE_STOP_PENDING);
  m_stopEvent.Set();
}

void ServiceExecutor::OnPause() {
  GLOG_INFO("Pause");
}

void ServiceExecutor::OnContinue() {
  GLOG_INFO("Continue");
}

void ServiceExecutor::OnInterrogate() {
  GLOG_INFO("Interrogate");
}

void ServiceExecutor::OnShutdown() {
  GLOG_INFO("Shutdown");
  m_status.Update(SERVICE_STOP_PENDING);
  m_stopEvent.Set();
}

void ServiceExecutor::OnHardwareProfileChange() {
  GLOG_INFO("OnHardwareProfileChange");
  m_controlListener->OnHardwareProfileChange();
}

void ServiceExecutor::OnPowerEvent() {
  GLOG_INFO("PowerEvent");
  m_controlListener->OnPowerEvent();
}

void ServiceExecutor::OnSessionChange(std::uint32_t reason,
                                      WTSSESSION_NOTIFICATION* notification) {
  GLOG_INFO("OnSessionChange, sessionId: %d", notification->dwSessionId);
  m_controlListener->OnSessionChange(reason, notification->dwSessionId);
}

ServiceExecutor::SWindowsServiceInstallCommand::SWindowsServiceInstallCommand()
    : CmdProcessorBase(L"install") {}

bool ServiceExecutor::SWindowsServiceInstallCommand::Process(void* args,
                                                             bool* success) {
  std::wstring name;
  m_entry->GetAppName(&name);
  return ServiceUtils::InstallService(name);
}

ServiceExecutor::SWindowsServiceUninstallCommand::
    SWindowsServiceUninstallCommand()
    : CmdProcessorBase(L"uninstall") {}

bool ServiceExecutor::SWindowsServiceUninstallCommand::Process(void* args,
                                                               bool* success) {
  std::wstring name;
  m_entry->GetAppName(&name);
  return ServiceUtils::UninstallService(name);
}

ServiceExecutor::SWindowsServiceStartCommand::SWindowsServiceStartCommand()
    : CmdProcessorBase(L"start") {}

bool ServiceExecutor::SWindowsServiceStartCommand::Process(void* args,
                                                           bool* success) {
  std::wstring name;
  m_entry->GetAppName(&name);
  return ServiceUtils::RunService(name);
}

ServiceExecutor::SWindowsServiceStopCommand::SWindowsServiceStopCommand()
    : CmdProcessorBase(L"stop") {}

bool ServiceExecutor::SWindowsServiceStopCommand::Process(void* args,
                                                          bool* success) {
  std::wstring name;
  m_entry->GetAppName(&name);
  return ServiceUtils::StopService(name);
}

ServiceExecutor::SWindowsServiceDebugCommand::SWindowsServiceDebugCommand()
    : CmdProcessorBase(L"debug") {}

bool ServiceExecutor::SWindowsServiceDebugCommand::Process(void* args,
                                                           bool* success) {
  assert(args);
  assert(success);
  auto debugExecutor = std::make_unique<SimpleExecutor>(*m_entry);
  debugExecutor->Execute(args);
  *success = true;
  return true;
}

std::unique_ptr<IExecutor> makeServiceExecutor(IEntry& entry) {
  return std::make_unique<ServiceExecutor>(entry);
}

}  // namespace Entries
}  // namespace Gengine