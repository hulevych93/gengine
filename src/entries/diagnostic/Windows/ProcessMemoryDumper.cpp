#include "ProcessMemoryDumper.h"

#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <tchar.h>
#include <dbghelp.h>

#include <boost/format.hpp>
#include <boost/stacktrace.hpp>

#include <core/Encoding.h>
#include <core/Logger.h>
#include <filesystem/Filesystem.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

typedef BOOL(WINAPI* MINIDUMPWRITEDUMP)(
    HANDLE hProcess,
    DWORD dwPid,
    HANDLE hFile,
    MINIDUMP_TYPE DumpType,
    CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
    CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
    CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

namespace Gengine {
namespace Diagnostic {
ProcessMemoryDumper* ProcessMemoryDumper::m_instance = nullptr;

const std::unordered_map<DumpType, std::string>
    ProcessMemoryDumper::DumpNamesMap = {
        {DumpType::Crash, "%1%_%2%_crash.dmp"},
        {DumpType::ExitLock, "%1%_%2%_exit_lock.dmp"}};

const std::wstring ProcessMemoryDumper::DebugHelpLib(L"dbghelp.dll");

std::string return_current_time_and_date() {
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);

  std::stringstream ss;
  ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
  return ss.str();
}

ProcessMemoryDumper::ProcessMemoryDumper(const std::string& name,
                                         bool blockThread) {
  // if this assert fires then you have two instances of MiniDumper
  // which is not allowed
  assert(m_instance == nullptr);
  m_appName = name;
  m_block = blockThread;
  m_instance = this;

  ::SetUnhandledExceptionFilter(TopLevelFilter);
}

LONG ProcessMemoryDumper::WriteDump(
    DumpType type,
    struct _EXCEPTION_POINTERS* pExceptionInfo) {
  DumpStack();

  std::string message;
  auto result = EXCEPTION_CONTINUE_SEARCH;

  auto dbgHelpPath =
      Filesystem::CombinePath(Filesystem::GetAppFolder(), DebugHelpLib);
  auto hDll = ::LoadLibraryW(dbgHelpPath.c_str());
  if (hDll == nullptr) {
    hDll = ::LoadLibraryW(DebugHelpLib.c_str());
  }
  if (hDll) {
    auto pDump = reinterpret_cast<MINIDUMPWRITEDUMP>(
        ::GetProcAddress(hDll, "MiniDumpWriteDump"));
    if (pDump) {
      auto fileName = GetDumpFileName(type);
      auto filePath = Filesystem::CombinePath(Filesystem::GetAppFolder(),
                                              utf8toWchar(fileName));

      auto hFile =
          ::CreateFileW(filePath.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE,
                        nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
      if (hFile != INVALID_HANDLE_VALUE) {
        BOOL bOK = FALSE;
        if (pExceptionInfo) {
          _MINIDUMP_EXCEPTION_INFORMATION ExInfo;
          ExInfo.ThreadId = ::GetCurrentThreadId();
          ExInfo.ExceptionPointers = pExceptionInfo;
          ExInfo.ClientPointers = 0;
          bOK = pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile,
                      MiniDumpNormal, &ExInfo, nullptr, nullptr);
        } else {
          bOK = pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile,
              MiniDumpNormal, nullptr, nullptr, nullptr);
        }

        if (bOK) {
          result = EXCEPTION_EXECUTE_HANDLER;
        }

        ::CloseHandle(hFile);
      }
    }
  }

  if (m_block) {
    ::MessageBoxA(nullptr, message.c_str(), m_instance->m_appName.c_str(),
                  MB_OK);
  }

  return result;
}

void ProcessMemoryDumper::WriteDump() {
  WriteDump(DumpType::ExitLock);
}

std::string ProcessMemoryDumper::GetDumpFileName(DumpType type) {
  std::string name;
  try {
    auto faceIter = DumpNamesMap.find(type);
    if (faceIter != DumpNamesMap.end()) {
      name = (boost::format(faceIter->second) % return_current_time_and_date() %
              m_appName)
                 .str();
    }
    std::replace(name.begin(), name.end(), ':', '-');
    std::replace(name.begin(), name.end(), ' ', '_');
  } catch (const std::exception&) {
  }

  return name;
}

void ProcessMemoryDumper::DumpStack() {
  std::ostringstream stackstream;
  stackstream << boost::stacktrace::stacktrace(0, 32);
  GLOG_ERROR_INTERNAL("Backtrace:%s\n", stackstream.str());
}

LONG ProcessMemoryDumper::TopLevelFilter(
    struct _EXCEPTION_POINTERS* pExceptionInfo) {
  return m_instance->WriteDump(DumpType::Crash, pExceptionInfo);
}
}  // namespace Diagnostic
}  // namespace Gengine