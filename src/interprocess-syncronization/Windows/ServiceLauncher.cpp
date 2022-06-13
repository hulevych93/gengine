#include "ServiceLauncher.h"

#include <core/Logger.h>
#include <entries/Windows/ServiceUtils.h>

namespace Gengine {
namespace InterprocessSynchronization {

using namespace AppConfig;
using namespace Services;

ServiceLauncher::ServiceLauncher(const std::wstring& serviceName)
    : Worker(1),
      m_serviceName(serviceName),
      m_checkAppsTimerId(Services::InvalidTimerID) {}

void ServiceLauncher::StartInternal() {
  if (m_checkAppsTimerId == Services::InvalidTimerID) {
    auto handler = boost::bind(&ServiceLauncher::CheckServicesRoutine, this);
	m_checkAppsTimerId = GENGINE_START_TIMER(handler, std::chrono::seconds{ 3 });
  }
}

void ServiceLauncher::StopInternal() {
  if (m_checkAppsTimerId != Services::InvalidTimerID) {
    GENGINE_STOP_TIMER_WITH_WAIT(m_checkAppsTimerId);
    m_checkAppsTimerId = Services::InvalidTimerID;
  }

  ServiceUtils::StopService(m_serviceName);
}

void ServiceLauncher::CheckServicesRoutine() {
  if (!ServiceUtils::IsServiceRunning(m_serviceName)) {
    ServiceUtils::RunService(m_serviceName);
  }
}

bool ServiceLauncher::IsCanStart() {
  return ServiceUtils::IsServiceInstalled(m_serviceName);
}
}  // namespace InterprocessSynchronization
}  // namespace Gengine