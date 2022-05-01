#pragma once

#include <string>
#include <memory>
#include <vector>
#include <interprocess-syncronization/InstanceRegistratorInterface.h>
#include <interprocess-syncronization/ServiceTracker.h>

namespace Gengine {
namespace Diagnostic {
class IDumper;
}

namespace InterprocessSynchronization {
class ServiceTracker;
class IAliveObject;
}

namespace Entries {
class IEntryToolsFactory
{
public:
    virtual ~IEntryToolsFactory() = default;
    virtual std::vector<std::unique_ptr<Diagnostic::IDumper>> CreateDumpers(const std::wstring& module) = 0;
    virtual std::unique_ptr<InterprocessSynchronization::IAliveObject> CreateAliveObject(const std::wstring& module) = 0;
    virtual std::unique_ptr<InterprocessSynchronization::InstanceRegistratorInterface> CreateInstanceRegistrator(const std::wstring& module, InterprocessSynchronization::InstanceType type = InterprocessSynchronization::InstanceType::Sigle) = 0;
    virtual std::unique_ptr<InterprocessSynchronization::ServiceTracker> CreateModuleTracker(const std::wstring& module, InterprocessSynchronization::ServiceTracker::terminate_handler handler) = 0;
};
}
}