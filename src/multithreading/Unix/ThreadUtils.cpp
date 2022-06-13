
#include <multithreading/ThreadUtils.h>

namespace Gengine {
namespace Multithreading {

bool SetThreadName(std::thread&, const std::string&) {
  return false;
}

std::uint32_t WaitForEventsEx(void*, std::uint32_t) {
  return -1;
}

}  // namespace Multithreading
}  // namespace Gengine
