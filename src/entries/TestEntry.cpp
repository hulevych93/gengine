#include <entries/TestEntry.h>
#include <entries/TestArgsVisitor.h>
#include <entries/CoreDefs.h>

#include <gtest/gtest.h>
#include <core/Logger.h>

namespace Gengine {
namespace Entries {

GTestModule::GTestModule(std::unique_ptr<IEntryToolsFactory>&& factory)
    : MasterEntry(std::move(factory))
{}

bool GTestModule::Execute(void* args)
{
    try
    {
        boost::apply_visitor(gtestArgsVisitor(), *reinterpret_cast<args_type*>(args));
    }
    catch (const std::exception& ex)
    {
        GLOG_ERROR("Failed to execute: %s", ex.what());
    }
    return true;
}

bool GTestModule::Exit(std::int32_t* exitCode)
{
    *exitCode = RUN_ALL_TESTS();
    return true;
}

bool GTestModule::CreateExecutor(std::shared_ptr<IExecutor>* executor)
{
    assert(executor);
    *executor = std::make_unique<SimpleExecutor>(*this);
    return true;
}

bool GTestModule::CreateProcessors(std::vector<std::unique_ptr<ICmdProcessor>>*)
{
    return true;
}

}
}
