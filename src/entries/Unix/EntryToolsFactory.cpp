#include "EntryToolsFactory.h"

#include <entries/diagnostic/Unix/LinuxCallstackDumper.h>
#include <entries/diagnostic/Unix/LinuxErrorsDumper.h>
#include <interprocess-synchronization/Unix/DaemonAliveObject.h>
#include <interprocess-synchronization/Unix/LinuxDaemonTracker.h>
#include <interprocess-synchronization/Unix/LinuxSingleInstanceRegistrator.h>
#include <boost/format.hpp>

#include <core/Encoding.h>
#include <filesystem/Filesystem.h>

namespace Gengine {
using namespace Diagnostic;
using namespace InterprocessSynchronization;

namespace Entries {
std::vector<std::unique_ptr<IDumper>> EntryToolsFactory::CreateDumpers(
    const std::wstring& module) {
  std::vector<std::unique_ptr<Diagnostic::IDumper>> dumpers;
  dumpers.emplace_back(std::make_unique<LinuxCallstackDumper>());
  dumpers.emplace_back(std::make_unique<LinuxErrorsDumper>());
  return dumpers;
}

std::unique_ptr<IAliveObject> EntryToolsFactory::CreateAliveObject(
    const std::wstring& module) {
  return std::make_unique<DaemonAliveObject>(module.c_str());
}

std::unique_ptr<InstanceRegistratorInterface>
EntryToolsFactory::CreateInstanceRegistrator(const std::wstring& module,
                                             InstanceType type) {
  std::wstring objName;

  switch (type) {
    case InstanceType::Sigle:
      objName += module;
      break;
    case InstanceType::OnePerUserSession:
      // objName += Filesystem::GetKernelObjectPath(boost::wformat(module) %
      // utf8toWchar(ProcessHelper::GetSessionKey(ProcessHelper::GetCurrentProcessId()))).str());
      break;
  }

  if (!objName.empty()) {
    return std::make_unique<LinuxSingleInstanceRegistrator>(std::move(objName));
  }
  return std::unique_ptr<InstanceRegistratorInterface>();
}

std::unique_ptr<ServiceTracker> EntryToolsFactory::CreateModuleTracker(
    const std::wstring& module,
    InterprocessSynchronization::ServiceTracker::terminate_handler handler) {
  return std::make_unique<LinuxDaemonTracker>(toUtf8(module), handler);
}

std::unique_ptr<IEntryToolsFactory> makeFactory() {
  return std::make_unique<EntryToolsFactory>();
}

}  // namespace Entries
}  // namespace Gengine
