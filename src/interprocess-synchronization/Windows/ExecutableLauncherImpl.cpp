#include "ExecutableLauncherImpl.h"

#include <Windows.h>

#include <multithreading/ThreadUtils.h>
#include <core/Logger.h>

namespace Gengine {
namespace InterprocessSynchronization {
using namespace Services;
using namespace AppConfig;

ExecutableLauncherImpl::ExecutableLauncherImpl()
    : ExecutableLauncher(),
      m_serviceObj("AAA", *this),
      m_eventPool(2),
      m_loopId(InvalidTimerID) {
  m_stopEvent = CreateEvent(NULL, true, false, NULL);
  m_changeEvent = CreateEvent(NULL, true, false, NULL);

  m_eventPool[0] = reinterpret_cast<HANDLE>(m_stopEvent);
  m_eventPool[1] = reinterpret_cast<HANDLE>(m_changeEvent);
}

ExecutableLauncherImpl::~ExecutableLauncherImpl() {
	::CloseHandle(m_stopEvent);
	::CloseHandle(m_changeEvent);
}

void ExecutableLauncherImpl::AddExecutable(
    const executable_params& params,
    IExecutableLauncherListener& listener) {
  SetEvent(m_changeEvent);
  ExecutableLauncher::AddExecutable(params, listener);
  ResetEvent(m_changeEvent);
}

void ExecutableLauncherImpl::RemoveExecutable(const executable_params& params) {
  SetEvent(m_changeEvent);
  ExecutableLauncher::RemoveExecutable(params);
  ResetEvent(m_changeEvent);
}

void ExecutableLauncherImpl::StartInternal() {
  m_serviceObj.Reveal();

  auto handler = [this] { CheckAppsRoutine(); };
  GENGINE_POST_TASK(handler);

  if (m_loopId == InvalidTimerID) {
    auto handler = [this] { Loop(); };
    m_loopId = GENGINE_START_TIMER(handler, std::chrono::milliseconds(500));
  }
}

void ExecutableLauncherImpl::StopInternal() {
  m_serviceObj.Hide();

  if (m_loopId != InvalidTimerID) {
    SetEvent(m_stopEvent);
    GENGINE_STOP_TIMER_WITH_WAIT(m_loopId);
    m_loopId = InvalidTimerID;
  }

  ExecutableLauncher::StopInternal();
}

void ExecutableLauncherImpl::Loop() {
  auto waitStatus = Multithreading::WaitForEventsEx(&m_eventPool[0], m_eventPool.size());
  switch (waitStatus) {
    case WAIT_OBJECT_0:
      break;
    case WAIT_OBJECT_0 + 1:
      break;
    case WAIT_FAILED:
      GLOG_ERROR("Wait events failed! Error: %d", GetLastError());
      break;
    case WAIT_TIMEOUT:
      GLOG_ERROR("Timeout in infinite mode? Is it possible?");
      break;
    default: {
      CheckAppsRoutine();
    } break;
  }
}

bool ExecutableLauncherImpl::OnHardwareProfileChange() {
  return true;
}

bool ExecutableLauncherImpl::OnPowerEvent() {
  return true;
}

bool ExecutableLauncherImpl::OnSessionChange(std::uint32_t event,
                                             std::uint32_t sessionId) {
  return true;
}

void ExecutableLauncherImpl::OnExecutableLaunched(
    const std::shared_ptr<Executable>& app) {
  auto index = m_eventPool.size();
  m_eventPool.resize(index + 1);

  m_eventPool.at(index) = app->GetHandle();
}

void ExecutableLauncherImpl::OnExecutableClosed(
    const std::shared_ptr<Executable>& app) {
  auto handleIter =
      std::find(m_eventPool.begin(), m_eventPool.end(), app->GetHandle());
  if (handleIter != m_eventPool.end()) {
    m_eventPool.erase(handleIter);
  }
}

}  // namespace InterprocessSynchronization
}  // namespace Gengine