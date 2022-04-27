#pragma once

#include <entries/IEntryToolsFactory.h>

namespace Gengine {
namespace Entries {
class LinuxEntryToolsFactory: public IEntryToolsFactory
{
public:
    std::vector<std::unique_ptr<Diagnostic::IDumper>> CreateDumpers(const std::wstring& module) override;
    std::unique_ptr<InterprocessSynchronization::IAliveObject> CreateAliveObject(const std::wstring& module) override;
    std::unique_ptr<InterprocessSynchronization::InstanceRegistratorInterface> CreateInstanceRegistrator(const std::wstring& module, InterprocessSynchronization::InstanceType type = InterprocessSynchronization::InstanceType::Sigle) override;
    std::unique_ptr<InterprocessSynchronization::ServiceTracker> CreateModuleTracker(const std::wstring& module, InterprocessSynchronization::ServiceTrackerImpl& serviceTrackerImpl) override;
};
}
}
