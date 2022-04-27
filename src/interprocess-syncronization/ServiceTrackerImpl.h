#pragma once

#include <memory>

namespace Gengine {
namespace InterprocessSynchronization {
class ServiceTrackerImpl
{
public:
    virtual void Terminate() = 0;
};

using TServiceTrackerImpl = std::shared_ptr<ServiceTrackerImpl>;
}
}
