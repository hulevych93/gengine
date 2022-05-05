#include "ExecutableLauncherImpl.h"

#include <Windows.h>
#include <core/Logger.h>

namespace Gengine {
namespace InterprocessSynchronization {
using namespace Multithreading;
using namespace Services;
using namespace AppConfig;

ExecutableLauncherImpl::ExecutableLauncherImpl()
    : ExecutableLauncher(),
      m_serviceObj("AAA", *this),
      m_eventPool(2),
      m_loopId(INVALID_TIMER_ID) {
  m_StopEvent.Create(true, false);
  m_ChangeEvent.Create(true, false);
  m_eventPool[0] = reinterpret_cast<HANDLE>(m_StopEvent.GetOSHandle());
  m_eventPool[1] = reinterpret_cast<HANDLE>(m_ChangeEvent.GetOSHandle());
}

void ExecutableLauncherImpl::AddExecutable(
    const executable_params& params,
    IExecutableLauncherListener& listener) {
  m_ChangeEvent.Set();
  ExecutableLauncher::AddExecutable(params, listener);
  m_ChangeEvent.Reset();
}

void ExecutableLauncherImpl::RemoveExecutable(const executable_params& params) {
  m_ChangeEvent.Set();
  ExecutableLauncher::RemoveExecutable(params);
  m_ChangeEvent.Reset();
}

void ExecutableLauncherImpl::StartInternal() {
  m_serviceObj.Reveal();

  auto handler = [this] { CheckAppsRoutine(); };
  GENGINE_POST_TASK(handler);

  if (m_loopId == INVALID_TIMER_ID) {
    auto handler = [this] { Loop(); };
    m_loopId = GENGINE_START_TIMER(handler, 500);
  }
}

void ExecutableLauncherImpl::StopInternal() {
  m_serviceObj.Hide();

  if (m_loopId != INVALID_TIMER_ID) {
    m_StopEvent.Set();
    GENGINE_STOP_TIMER_WITH_WAIT(m_loopId);
    m_loopId = INVALID_TIMER_ID;
  }

  ExecutableLauncher::StopInternal();
}

void ExecutableLauncherImpl::Loop() {
  auto waitStatus = Event::WaitForEventsEx(&m_eventPool[0], m_eventPool.size());
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