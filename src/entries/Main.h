#pragma once

#include <memory>
#include <unordered_map>

#include <entries/BaseArgs.h>
#include <entries/EntryPoint.h>
#include <entries/EntryBase.h>

#include <entries/IEntryToolsFactory.h>
#include <api/entries/ICmdProcessor.h>
#include <api/entries/IEntry.h>
#include <entries/EntryRegistry.h>
#include <entries/SimpleExecutor.h>

namespace Gengine {
namespace Entries {
class Main
{
public:
    Main();

    Main(const Main&) = delete;
    Main(Main&&) = delete;

    Main& operator=(const Main&) = delete;
    Main& operator=(Main&&) = delete;

    ~Main() = default;

public:
    std::int32_t Run(args_type args = args_type());

protected:
    void RegisterProcessor(std::unique_ptr<ICmdProcessor>&& processor);
    bool ProcessCmdLine(args_type args, bool* result) const;
    std::shared_ptr<IEntry> GetEntry(args_type args) const;

protected:
    using ProcessorsMap = std::unordered_map<std::wstring, std::unique_ptr<ICmdProcessor>>;

    mutable std::unique_ptr<IEntryToolsFactory> m_factory;
    mutable std::shared_ptr<IEntry> m_entry;
    ProcessorsMap m_optionsProcessors;

private:
    static Main* Instance;
    static const std::wstring ProgramOptionsPattern;
};
}
}
