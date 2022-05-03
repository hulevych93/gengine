#include "Future.h"

namespace Gengine {
namespace Multithreading {
Future::Future(bool processing) : m_canceled(false), m_processing(processing) {
  m_completed.Create(true, false);
}

Future::~Future() {}

void Future::Wait(std::uint32_t timeout) {
  if (m_processing) {
    m_completed.Wait(timeout);
  }
}

bool Future::IsCanceled() const {
  return m_canceled;
}

void Future::Cancel() {
  m_canceled = true;
  m_completed.Set();
}

void Future::Complete() {
  m_completed.Set();
  m_processing = false;
}

void Future::Reset() {
  m_processing = true;
  m_completed.Reset();
}
}  // namespace Multithreading
}  // namespace Gengine