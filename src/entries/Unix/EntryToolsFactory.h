#pragma once

#include <entries/IEntryToolsFactory.h>

namespace Gengine {
namespace Entries {
class EntryToolsFactory : public IEntryToolsFactory {
 public:
  std::vector<std::unique_ptr<Diagnostic::IDumper>> CreateDumpers(
      const std::wstring& module) override;
  std::unique_ptr<InterprocessSynchronization::IAliveObject> CreateAliveObject(
      const std::wstring& module) override;
  std::unique_ptr<InterprocessSynchronization::InstanceRegistratorInterface>
  CreateInstanceRegistrator(
      const std::wstring& module,
      InterprocessSynchronization::InstanceType type =
          InterprocessSynchronization::InstanceType::Sigle) override;
  std::unique_ptr<InterprocessSynchronization::ServiceTracker>
  CreateModuleTracker(
      const std::wstring& module,
      InterprocessSynchronization::ServiceTracker::terminate_handler
          serviceTrackerImpl) override;
};
}  // namespace Entries
}  // namespace Gengine
