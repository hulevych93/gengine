#pragma once

#include <interprocess-syncronization/InstanceRegistratorInterface.h>
#include <interprocess-syncronization/ServiceTracker.h>
#include <memory>
#include <string>
#include <vector>

namespace Gengine {
namespace Diagnostic {
class IDumper;
}

namespace InterprocessSynchronization {
class ServiceTracker;
class IAliveObject;
}  // namespace InterprocessSynchronization

namespace Entries {
class IEntryToolsFactory {
 public:
  virtual ~IEntryToolsFactory() = default;
  virtual std::vector<std::unique_ptr<Diagnostic::IDumper>> CreateDumpers(
      const std::wstring& module) = 0;
  virtual std::unique_ptr<InterprocessSynchronization::IAliveObject>
  CreateAliveObject(const std::wstring& module) = 0;
  virtual std::unique_ptr<
      InterprocessSynchronization::InstanceRegistratorInterface>
  CreateInstanceRegistrator(
      const std::wstring& module,
      InterprocessSynchronization::InstanceType type =
          InterprocessSynchronization::InstanceType::Sigle) = 0;
  virtual std::unique_ptr<InterprocessSynchronization::ServiceTracker>
  CreateModuleTracker(
      const std::wstring& module,
      InterprocessSynchronization::ServiceTracker::terminate_handler
          handler) = 0;
};

std::unique_ptr<IEntryToolsFactory> makeFactory();

}  // namespace Entries
}  // namespace Gengine
