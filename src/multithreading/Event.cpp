#include <multithreading/Event.h>

namespace Gengine {
namespace Multithreading {

Event::Event() : m_manualReset(false), m_signaled(false) {}

Event::Event(ManualResetTag) : m_manualReset(true), m_signaled(false) {}

void Event::Set() {
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_signaled = true;
  }
  m_cond.notify_all();
}

void Event::Reset() {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_signaled = false;
}

bool Event::Wait(const std::chrono::system_clock::duration timeout) {
  bool signaled = true;

  std::unique_lock<std::mutex> lock(m_mutex);

  if (!m_signaled) {
    if (timeout != std::chrono::system_clock::duration::max()) {
      signaled = m_cond.wait_for(lock, timeout, [&]() { return m_signaled; });
    } else {
      m_cond.wait(lock, [&]() { return m_signaled; });
    }
  }

  if (!m_manualReset) {
    m_signaled = false;
  }

  return signaled;
}

}  // namespace Multithreading
}  // namespace Gengine
