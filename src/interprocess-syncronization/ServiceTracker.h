#pragma once

#include <string>
#include <memory>
#include <functional>

#include <multithreading/Event.h>
#include <core/Runnable.h>


namespace Gengine {
namespace InterprocessSynchronization {
class ServiceTracker : public Runnable
{
public:
    using terminate_handler = std::function<void()>;

    ServiceTracker(const ServiceTracker&) = delete;
    virtual ~ServiceTracker() = default;

    void Terminate();

protected:
    ServiceTracker(terminate_handler serviceTrackerImpl);

private:
    terminate_handler m_handler;
};
}
}