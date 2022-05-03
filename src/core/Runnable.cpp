#include <core/Runnable.h>

namespace Gengine {
Runnable::Runnable() : m_isRunning(false) {}

bool Runnable::Start() {
  if (IsCanStart()) {
    auto running = false;
    IsRunning(&running);
    if (!running) {
      m_isRunning.store(true);
      StartInternal();
      m_startedSignal();
    }
  }

  return true;
}

bool Runnable::Stop() {
  auto running = false;
  IsRunning(&running);
  if (running) {
    m_isRunning.store(false);
    StopInternal();
    m_stoppedSignal();
  }

  return true;
}

bool Runnable::IsRunning(bool* running) {
  *running = m_isRunning.load();
  return true;
}

Runnable::connection Runnable::AddStartedListener(
    signal::slot_function_type slot) {
  return m_startedSignal.connect(slot);
}

Runnable::connection Runnable::AddStoppedListener(
    signal::slot_function_type slot) {
  return m_stoppedSignal.connect(slot);
}

bool Runnable::IsCanStart() {
  return true;
}

}  // namespace Gengine