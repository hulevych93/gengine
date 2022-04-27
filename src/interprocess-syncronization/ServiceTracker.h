#pragma once

#include <string>
#include <memory>

#include <multithreading/Event.h>
#include <interprocess-syncronization/ServiceTrackerImpl.h>
#include <core/Runnable.h>

namespace Gengine {
namespace InterprocessSynchronization {
class ServiceTracker : public Runnable
{
public:
    ServiceTracker(const ServiceTracker&) = delete;
    virtual ~ServiceTracker() = default;

    void Terminate();

protected:
    ServiceTracker(ServiceTrackerImpl& serviceTrackerImpl);

private:
    ServiceTrackerImpl& m_serviceTrackerImpl;
};
}
}