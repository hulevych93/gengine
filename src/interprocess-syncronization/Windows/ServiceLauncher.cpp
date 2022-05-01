#include "ServiceLauncher.h"

#include <entries/Windows/ServiceUtils.h>
#include <core/Logger.h>

namespace Gengine {
namespace InterprocessSynchronization {

using namespace AppConfig;
using namespace Services;

const std::uint32_t ServiceLauncher::CheckTimeout(3000); // 3 sec

ServiceLauncher::ServiceLauncher(const std::wstring& serviceName)
    : Worker(1)
    , m_serviceName(serviceName)
    , m_checkAppsTimerId(Services::INVALID_TIMER_ID)
{}

void ServiceLauncher::StartInternal()
{
    if (m_checkAppsTimerId == Services::INVALID_TIMER_ID)
    {
        auto handler = boost::bind(&ServiceLauncher::CheckServicesRoutine, this);
        m_checkAppsTimerId = START_HEARTBEAT_TIMER(handler, CheckTimeout);
    }
}

void ServiceLauncher::StopInternal()
{
    if (m_checkAppsTimerId != Services::INVALID_TIMER_ID)
    {
        STOP_HEARTBEAT_TIMER_WITH_WAIT(m_checkAppsTimerId);
        m_checkAppsTimerId = Services::INVALID_TIMER_ID;
    }

    ServiceUtils::StopService(m_serviceName);
}

void ServiceLauncher::CheckServicesRoutine()
{
    if (!ServiceUtils::IsServiceRunning(m_serviceName))
    {
        ServiceUtils::RunService(m_serviceName);
    }
}

bool ServiceLauncher::IsCanStart()
{
    return ServiceUtils::IsServiceInstalled(m_serviceName);
}
}
}