#pragma once

#include <api/core/IRunnable.h>
#include <atomic>
#include <boost/signals2.hpp>

namespace Gengine {

/*
 * @class Runnable absract class is the common type for runnable object types.
 * As well as monitoring of the state of object (e.g. running/stopped) user
 * can subscribe and receive notifications abount start/stop actions of the
 * runnable.
 */
class Runnable : public IRunnable {
 public:
  using connection = boost::signals2::connection;
  using signal = boost::signals2::signal<void()>;

 public:
  /*
   * Default constructor makes not running object.
   *
   */
  Runnable();

  /*
   * Start, IsRunning, Stop are methods for running object's state manipulation.
   * @param[out] running says if object's state is running.
   * @return true if the operation is performed.
   */
  bool Start() override;
  bool IsRunning(bool* running) override;
  bool Stop() override;

  /*
   *  These functions are for start/stop listeners.
   *  @param slot can be eather lamda expressions or some function binded via
   * std::bind.
   *  @return connection object which is a RAII type, as far as it alive the
   * connection is not broken.
   */
  connection AddStartedListener(signal::slot_function_type slot);
  connection AddStoppedListener(signal::slot_function_type slot);

 protected:
  virtual void StartInternal() = 0;
  virtual void StopInternal() = 0;
  virtual bool IsCanStart();

 private:
  std::atomic<bool> m_isRunning;

 private:
  signal m_startedSignal;
  signal m_stoppedSignal;
};
}  // namespace Gengine
