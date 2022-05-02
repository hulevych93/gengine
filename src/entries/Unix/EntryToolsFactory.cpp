#include "EntryToolsFactory.h"

#include <boost/format.hpp>
#include <diagnostic/Unix/LinuxErrorsDumper.h>
#include <diagnostic/Unix/LinuxCallstackDumper.h>
#include <interprocess-syncronization/Unix/DaemonAliveObject.h>
#include <interprocess-syncronization/Unix/LinuxSingleInstanceRegistrator.h>
#include <interprocess-syncronization/Unix/LinuxDaemonTracker.h>

#include <filesystem/Filesystem.h>
#include <core/Encoding.h>

namespace Gengine {
using namespace Diagnostic;
using namespace InterprocessSynchronization;

namespace Entries {
std::vector<std::unique_ptr<IDumper>> EntryToolsFactory::CreateDumpers(const std::wstring& module)
{
    std::vector<std::unique_ptr<Diagnostic::IDumper>> dumpers;
    dumpers.emplace_back(std::make_unique<LinuxCallstackDumper>());
    dumpers.emplace_back(std::make_unique<LinuxErrorsDumper>());
    return dumpers;
}

std::unique_ptr<IAliveObject> EntryToolsFactory::CreateAliveObject(const std::wstring& module)
{
    return std::make_unique<DaemonAliveObject>(module.c_str());
}

std::unique_ptr<InstanceRegistratorInterface> EntryToolsFactory::CreateInstanceRegistrator(const std::wstring& module, InstanceType type)
{
    std::wstring objName;

    switch(type)
    {
    case InstanceType::Sigle:
        objName += module;
        break;
    case InstanceType::OnePerUserSession:
        //objName += Filesystem::GetKernelObjectPath(boost::wformat(module) % utf8toWchar(ProcessHelper::GetSessionKey(ProcessHelper::GetCurrentProcessId()))).str());
        break;
    }

    if(!objName.empty())
    {
        return std::make_unique<LinuxSingleInstanceRegistrator>(std::move(objName));
    }
    return std::unique_ptr<InstanceRegistratorInterface>();
}

std::unique_ptr<ServiceTracker> EntryToolsFactory::CreateModuleTracker(const std::wstring& module,InterprocessSynchronization::ServiceTracker::terminate_handler handler)
{
    return std::make_unique<LinuxDaemonTracker>(toUtf8(module), handler);
}

std::unique_ptr<IEntryToolsFactory> makeFactory()
{
    return std::make_unique<EntryToolsFactory>();
}

}
}
