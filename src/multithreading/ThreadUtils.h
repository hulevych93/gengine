#pragma once

#include <string>
#include <thread>

namespace Gengine {
namespace Multithreading {
bool SetThreadName(std::thread& thread, const std::string& name);
}  // namespace Multithreading
}  // namespace Gengine
