
#include <multithreading/ThreadUtils.h>

namespace Gengine {
namespace Multithreading {

bool SetThreadName(std::thread&, const std::string&) {
  return false;
}

}  // namespace Multithreading
}  // namespace Gengine
