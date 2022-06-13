
#include <multithreading/ThreadUtils.h>
#include <core/Logger.h>

#include <thread>
#include <vector>
#include <memory>

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

bool SetThreadName(std::thread& thread, const std::string& name) {
  DWORD threadId = ::GetThreadId(static_cast<HANDLE>(thread.native_handle()));
  return ThreadName(threadId, name.c_str());
}

std::uint32_t WaitForEventsEx(void* events, std::uint32_t count) {
	DWORD waitResult = WAIT_FAILED;

	if (count <= MAXIMUM_WAIT_OBJECTS) {
		waitResult = WaitForMultipleObjects(
			count, reinterpret_cast<HANDLE*>(events), FALSE, INFINITE);
	}
	else {
		HANDLE waitEvent = CreateEvent(NULL, true, false, NULL);

		std::vector<std::thread> threads;
		std::vector<HANDLE*> waits;

		std::uint32_t currentIndex = 0;
		while (currentIndex < count) {
			std::uint32_t eventIndex = 0;
			std::uint32_t offset = currentIndex;

			auto subEvents = std::make_unique<HANDLE[]>(MAXIMUM_WAIT_OBJECTS);
			subEvents[eventIndex++] = waitEvent;

			while (eventIndex < MAXIMUM_WAIT_OBJECTS && currentIndex < count) {
				subEvents[eventIndex++] =
					reinterpret_cast<HANDLE*>(events)[currentIndex++];
			}

			waits.push_back(subEvents.get());

			auto handler = [eventIndex, subEvents = std::move(subEvents), offset, &waitResult,
				&waitEvent]() {
				DWORD result = WaitForMultipleObjects(
					eventIndex, reinterpret_cast<HANDLE*>(subEvents.get()), FALSE,
					INFINITE);
				switch (result) {
				case WAIT_OBJECT_0:
					return;
					break;
				case WAIT_FAILED:
					GLOG_ERROR("Wait events failed! Error: %d", GetLastError());
					waitResult = result;
					break;
				case WAIT_TIMEOUT:
					GLOG_ERROR("Timeout in infinite mode? Is it possible?");
					break;
				default: {
					waitResult = result + offset - 1;
				} break;
				}

				SetEvent(waitEvent);
			};

			threads.emplace_back(std::thread(std::move(handler)));
		}

		WaitForSingleObject(waitEvent, INFINITE);
		::CloseHandle(waitEvent);

		for (auto& thread : threads)
		{
			thread.join();
		}
	}

	return waitResult;
}

}  // namespace Multithreading
}  // namespace Gengine
