#pragma once

#include <string>
#include <thread>

namespace Gengine {
namespace Multithreading {
bool SetThreadName(std::thread& thread, const std::string& name);
std::uint32_t WaitForEventsEx(void* events, std::uint32_t count);
}  // namespace Multithreading
}  // namespace Gengine
