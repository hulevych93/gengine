#include "Windows/EntryToolsFactory.h"

#include <diagnostic\Windows/ProcessMemoryDumper.h>
#include <diagnostic\Windows\MemoryLeaksDumper.h>
#include <interprocess-syncronization\Windows/ServiceAliveObject.h>
#include <interprocess-syncronization\Windows/ServiceTrackerImpl.h>
#include <interprocess-syncronization\Windows/SingleInstanceRegistratorImpl.h>
#include <boost/format.hpp>
//#include <core/ProcessHelper.h>
#include <core/Encoding.h>

using namespace Gengine::Diagnostic;
using namespace Gengine::InterprocessSynchronization;

namespace Gengine {
namespace Entries {
std::vector<std::unique_ptr<Diagnostic::IDumper>>
EntryToolsFactory::CreateDumpers(const std::wstring& module) {
  std::vector<std::unique_ptr<Diagnostic::IDumper>> dumpers;
  dumpers.emplace_back(
      std::make_unique<ProcessMemoryDumper>(toUtf8(module), false));
  dumpers.emplace_back(std::make_unique<MemoryLeaksDumper>(toUtf8(module)));
  return dumpers;
}

std::unique_ptr<IAliveObject> EntryToolsFactory::CreateAliveObject(
    const std::wstring& module) {
  return std::make_unique<ServiceAliveObject>(module.c_str());
}

std::unique_ptr<InstanceRegistratorInterface>
EntryToolsFactory::CreateInstanceRegistrator(const std::wstring& module,
                                             InstanceType type) {
  std::unique_ptr<InstanceRegistratorInterface> registrator;

  std::wstring objectName;

  switch (type) {
    case InstanceType::Sigle:
      objectName = std::move(module);
      break;
    case InstanceType::OnePerUserSession:
      // objectName = (boost::wformat(module) %
      // ProcessHelper::GetSessionKey(ProcessHelper::GetCurrentProcessId())).str();
      break;
  }

  if (!objectName.empty()) {
    registrator =
        std::make_unique<SingleInstanceRegistratorImpl>(std::move(objectName));
  }

  return registrator;
}

std::unique_ptr<ServiceTracker> EntryToolsFactory::CreateModuleTracker(
    const std::wstring& module,
    InterprocessSynchronization::ServiceTracker::terminate_handler handler) {
  return std::make_unique<ServiceTrackerImpl>(module.c_str(), handler);
}

std::unique_ptr<IEntryToolsFactory> makeFactory() {
  return std::make_unique<EntryToolsFactory>();
}

}  // namespace Entries
}  // namespace Gengine