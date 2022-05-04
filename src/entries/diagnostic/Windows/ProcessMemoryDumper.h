#pragma once

#include <string>
#include <unordered_map>
#include <windows.h>
#include <dbghelp.h>

#include <diagnostic/CommonDiagnosticDefs.h>
#include <diagnostic/IDumper.h>

namespace Gengine {
namespace Diagnostic {
class ProcessMemoryDumper : public IDumper
{
public:
    ProcessMemoryDumper(const std::string& name, MINIDUMP_TYPE MinidumpType, bool blockThread);

private:
    static LONG WINAPI TopLevelFilter(struct _EXCEPTION_POINTERS *pExceptionInfo);

public:
    void WriteDump() override;

protected:
    LONG WriteDump(DumpType type, struct _EXCEPTION_POINTERS *pExceptionInfo = nullptr);

private:
    std::string GetDumpFileName(DumpType type);
    static void DumpStack();

private:
    std::string m_appName;
    MINIDUMP_TYPE m_minidumpType;
    bool m_block;

private:
    static ProcessMemoryDumper* m_instance;
    static const std::unordered_map<DumpType, std::string> DumpNamesMap;
    static const std::wstring DebugHelpLib;
};
}
}