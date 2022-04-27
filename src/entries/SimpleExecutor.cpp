#include <entries/SimpleExecutor.h>

#include <api/entries/IEntry.h>
#include <api/entries/ICmdProcessor.h>

namespace Gengine {
namespace Entries {
SimpleExecutor::SimpleExecutor(IEntry& entry)
    : m_entry(entry)
    , m_code(1)
{}

SimpleExecutor::~SimpleExecutor() = default;

bool SimpleExecutor::Execute(void* args)
{
    assert(args);
    if (m_entry.Initialize())
    {
        m_entry.Execute(args);
        m_entry.Exit(&m_code);
        m_entry.Finalize();
    }
    return true;
}

bool SimpleExecutor::GetCode(std::int32_t* exitCode)
{
    assert(exitCode);
    *exitCode = m_code;
    return true;
}

bool SimpleExecutor::CreateProcessors(std::vector<std::unique_ptr<ICmdProcessor>>* processors)
{
    return true;
}

IEntry& SimpleExecutor::GetEntry() const
{
    return m_entry;
}

}
}