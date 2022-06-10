#pragma once

#include <condition_variable>
#include <cstdint>
#include <mutex>

namespace Gengine {
namespace Multithreading {

/**
 * @brief The ManualResetTag struct
 *
 * Used as Event class constructor parameter tag.
 */
struct ManualResetTag final {};

/**
 * @brief The Event class
 *
 * The synchronization primitive with basic event set/reset semantics.
 */
class Event final {
 public:
  /**
   * @brief WaitInfinite
   */
  static const std::uint32_t WaitInfinite = -1;

 public:
  /**
   * @brief Event constructor with auto-reset.
   */
  Event();

  /**
   * Event constructor with manual-reset.
   */
  explicit Event(ManualResetTag);

  /**
   * @brief Set event ready.
   */
  void Set();

  /**
   * @brief Reset event.
   */
  void Reset();

  /**
   * @brief Wait for event.
   * @param mills is time in milliseconds.
   * @return true if wait succeded.
   */
  bool Wait(std::uint32_t mills);

 private:
  std::mutex m_mutex;
  std::condition_variable m_cond;

  bool m_manualReset;
  bool m_signaled;
};
}  // namespace Multithreading
}  // namespace Gengine
