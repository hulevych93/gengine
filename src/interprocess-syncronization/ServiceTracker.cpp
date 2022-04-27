#include <interprocess-syncronization/ServiceTracker.h>

namespace Gengine {
namespace InterprocessSynchronization {
ServiceTracker::ServiceTracker(ServiceTrackerImpl& serviceTrackerImpl)
    : m_serviceTrackerImpl(serviceTrackerImpl)
{}

void ServiceTracker::Terminate()
{
    m_serviceTrackerImpl.Terminate();
}
}
}