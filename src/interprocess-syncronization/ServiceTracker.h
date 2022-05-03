#pragma once

#include <functional>
#include <memory>
#include <string>

#include <core/Runnable.h>
#include <multithreading/Event.h>

namespace Gengine {
namespace InterprocessSynchronization {
class ServiceTracker : public Runnable {
 public:
  using terminate_handler = std::function<void()>;

  ServiceTracker(const ServiceTracker&) = delete;
  virtual ~ServiceTracker() = default;

  void Terminate();

 protected:
  ServiceTracker(terminate_handler serviceTrackerImpl);

 private:
  terminate_handler m_handler;
};
}  // namespace InterprocessSynchronization
}  // namespace Gengine