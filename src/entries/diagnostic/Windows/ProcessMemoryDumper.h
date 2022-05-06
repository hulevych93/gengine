#pragma once

#include <windows.h>
#include <string>
#include <unordered_map>

#include <diagnostic/CommonDiagnosticDefs.h>
#include <diagnostic/IDumper.h>

namespace Gengine {
namespace Diagnostic {
class ProcessMemoryDumper : public IDumper {
 public:
  ProcessMemoryDumper(const std::string& name, bool blockThread);

 private:
  static LONG WINAPI TopLevelFilter(struct _EXCEPTION_POINTERS* pExceptionInfo);

 public:
  void WriteDump() override;

 protected:
  LONG WriteDump(DumpType type,
                 struct _EXCEPTION_POINTERS* pExceptionInfo = nullptr);

 private:
  std::string GetDumpFileName(DumpType type);
  static void DumpStack();

 private:
  std::string m_appName;
  bool m_block;

 private:
  static ProcessMemoryDumper* m_instance;
  static const std::unordered_map<DumpType, std::string> DumpNamesMap;
  static const std::wstring DebugHelpLib;
};
}  // namespace Diagnostic
}  // namespace Gengine