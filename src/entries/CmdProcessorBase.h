#pragma once

#include <api/entries/ICmdProcessor.h>

namespace Gengine {
class IEntry;

namespace Entries {
class CmdProcessorBase : public ICmdProcessor
{
public:
    CmdProcessorBase(const std::wstring& command);
    CmdProcessorBase(const std::wstring& command, const std::wstring& description);
    virtual ~CmdProcessorBase();

    bool Register(void* options) override;
    bool SetEntry(void* entry) override;
    bool GetCommandKey(std::wstring* key) override;

protected:
    const std::wstring m_command;
    const std::wstring m_description;

protected:
    IEntry* m_entry;
};
}
}
