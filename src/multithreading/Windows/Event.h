#pragma once

#include <cstdint>

namespace Gengine {
namespace Multithreading {
class Event {
 public:
  Event();
  explicit Event(void* hEvent);
  ~Event();

  void* GetOSHandle();
  void Create(bool bManualReset, bool bInitialState);

  void Set();
  void Reset();
  bool Wait(std::uint32_t dwMilliseconds);

  static std::uint32_t WaitForEventsEx(void* events, std::uint32_t count);

 private:
  void* m_hEventHandle;
  bool m_bAttached;

 public:
  static const std::uint32_t WAIT_INFINITE = -1;
};
}  // namespace Multithreading
}  // namespace Gengine