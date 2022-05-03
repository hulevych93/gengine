#pragma once

#include <pthread.h>
#include <cstdint>
#include <list>
#include <mutex>

namespace Gengine {
namespace Multithreading {
class Event final {
 public:
  static const std::uint32_t WAIT_INFINITE = 0xFFFFFFFF;
  Event();
  ~Event();

  void Create(bool bManualReset, bool bInitialState);

  void Set();
  void Reset();
  // returns true if wait successfull or false if wait
  // exited by timeout
  // if dwMilliseconds==INFINITE
  // thread will wait finfinite for the event
  bool Wait(std::uint32_t dwMilliseconds);

  // returns number of event being setted or -1 if wait timeouted
  static int WaitMultiple(Event** ppEvents,
                          int iEventsCount,
                          bool bWaitAll,
                          std::uint32_t dwMilliseconds);

 private:
  struct mutex_cond_t {
    pthread_mutex_t m_Mutex;
    pthread_cond_t m_Cond;
  };
  // each thread, waiting for this event adds it mutex_cond_t
  // to the list
  std::list<mutex_cond_t*> m_MutexConditions;
  std::mutex m_csMutexConditions;
  void AddMutexCond(mutex_cond_t* pMutexCond);
  void RemoveMutexCond(mutex_cond_t* pMutexCond);

  bool m_bManualReset;
  bool m_bSignaled;
};
}  // namespace Multithreading
}  // namespace Gengine
