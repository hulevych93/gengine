#include <interprocess-syncronization/ServiceTracker.h>

namespace Gengine {
namespace InterprocessSynchronization {
ServiceTracker::ServiceTracker(terminate_handler handler)
    : m_handler(handler) {}

void ServiceTracker::Terminate() {
  m_handler();
}
}  // namespace InterprocessSynchronization
}  // namespace Gengine