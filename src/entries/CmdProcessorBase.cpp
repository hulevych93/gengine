#include "CmdProcessorBase.h"

#include <core/Encoding.h>
#include <boost/program_options.hpp>

namespace bpo = boost::program_options;

namespace Gengine {
namespace Entries {
CmdProcessorBase::CmdProcessorBase(const std::wstring& command)
    : m_command(command)
{}

CmdProcessorBase::CmdProcessorBase(const std::wstring& command, const std::wstring& description)
    : m_command(command)
    , m_description(description)
{}

CmdProcessorBase::~CmdProcessorBase() = default;

bool CmdProcessorBase::Register(void* options)
{
    assert(options);
    auto& programOptions = *reinterpret_cast<bpo::options_description_easy_init*>(options);
    programOptions(toUtf8(m_command).c_str(), toUtf8(m_description).c_str());
    return true;
}

bool CmdProcessorBase::SetEntry(void* entry)
{
    assert(entry);
    m_entry = reinterpret_cast<IEntry*>(entry);
    return true;
}

bool CmdProcessorBase::GetCommandKey(std::wstring* key)
{
    assert(key);
    *key = m_command;
    return true;
}

}
}