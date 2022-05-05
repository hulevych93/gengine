#include "Event.h"

#include <assert.h>
#include <core/Logger.h>
#include <windows.h>
#include <list>
#include <thread>

namespace Gengine {
namespace Multithreading {
Event::Event() : m_hEventHandle(NULL), m_bAttached(false) {}

Event::Event(void* hEvent)
    : m_hEventHandle((HANDLE)hEvent), m_bAttached(true) {}

void* Event::GetOSHandle() {
  return m_hEventHandle;
}

Event::~Event(void) {
  if (m_hEventHandle && !m_bAttached)
    CloseHandle(m_hEventHandle);
}

void Event::Create(bool bManualReset, bool bInitialState) {
  if (m_hEventHandle) {
    // don't create more then one time
    assert(0);
    return;
  }
  // create event
  m_hEventHandle = CreateEvent(NULL, bManualReset, bInitialState, NULL);

  // check event on validation
  assert(m_hEventHandle);
}

void Event::Set() {
  assert(m_hEventHandle);
  SetEvent(m_hEventHandle);
}

void Event::Reset() {
  assert(m_hEventHandle);
  ResetEvent(m_hEventHandle);
}

bool Event::Wait(std::uint32_t dwMilliseconds) {
  assert(m_hEventHandle);

  return (WaitForSingleObject(m_hEventHandle, dwMilliseconds) == WAIT_OBJECT_0);
}

std::uint32_t Event::WaitForEventsEx(void* events, std::uint32_t count) {
  DWORD waitResult = WAIT_FAILED;

  if (count <= MAXIMUM_WAIT_OBJECTS) {
    // IDubynets: that fine, we can use
    waitResult = WaitForMultipleObjects(
        count, reinterpret_cast<HANDLE*>(events), FALSE, INFINITE);
  } else {
    Event waitResultEvent;
    waitResultEvent.Create(true, false);

    // this is bad situation, we need to separate  waiting objects by threads
    std::vector<std::thread> threadsGroup;

    // we will put our stop event to all threads
    // in this case, all threads should be finished as soon as one event will
    // set
    HANDLE stopWaitEvent = waitResultEvent.GetOSHandle();

    std::list<HANDLE*> waitGroups;
    std::uint32_t currentIndex = 0;
    while (currentIndex < count) {
      int countInGroup = 0;
      int offset = currentIndex;

      HANDLE* eventGroup = new HANDLE[MAXIMUM_WAIT_OBJECTS];
      eventGroup[countInGroup++] = stopWaitEvent;

      while (countInGroup < MAXIMUM_WAIT_OBJECTS && currentIndex < count) {
        eventGroup[countInGroup++] =
            reinterpret_cast<HANDLE*>(events)[currentIndex++];
      }

      waitGroups.push_back(eventGroup);

      auto handler = [countInGroup, eventGroup, offset, &waitResult,
                      &waitResultEvent]() {
        DWORD result = WaitForMultipleObjects(
            countInGroup, reinterpret_cast<HANDLE*>(eventGroup), FALSE,
            INFINITE);
        if (result == WAIT_FAILED) {
          GLOG_ERROR("Wait for events failed! %d", GetLastError());
          waitResult = result;
        } else if (result == 0) {
          return;
        } else {
          waitResult = result + offset - 1;
        }

        waitResultEvent.Set();
      };

      threadsGroup.emplace_back(std::thread(handler));
    }

    waitResultEvent.Wait(Event::WAIT_INFINITE);

    // wait for finishing other threads
    std::for_each(threadsGroup.begin(), threadsGroup.end(),
                  std::bind(&std::thread::join, std::placeholders::_1));

    // remove blocks of data
    std::list<HANDLE*>::iterator it;
    for (it = waitGroups.begin(); it != waitGroups.end(); it++) {
      delete[] * it;
    }
  }

  return waitResult;
}
}  // namespace Multithreading
}  // namespace Gengine