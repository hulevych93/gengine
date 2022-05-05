#include <core/Encoding.h>
#include <multithreading/ThreadUtils.h>

#include <windows.h>
const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO {
  DWORD dwType;      // Must be 0x1000.
  LPCSTR szName;     // Pointer to name (in user addr space).
  DWORD dwThreadID;  // Thread ID (-1=caller thread).
  DWORD dwFlags;     // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

namespace Gengine {
namespace Multithreading {

bool ThreadName(DWORD dwThreadID, const char* threadName) {
  if (::IsDebuggerPresent() == TRUE) {
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;

#pragma warning(push)
#pragma warning(disable : 6320 6322)
    __try {
      RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR),
                     (ULONG_PTR*)&info);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
#pragma warning(pop)
    return true;
  }
  return false;
}

bool ThreadUtils::SetThreadName(std::thread& thread, const std::wstring& name) {
  DWORD threadId = ::GetThreadId(static_cast<HANDLE>(thread.native_handle()));
  return ThreadName(threadId, toUtf8(name).c_str());
}
}  // namespace Multithreading
}  // namespace Gengine