#pragma once

#include <string>
#include <thread>

namespace Gengine {
namespace Multithreading {
class ThreadUtils
{
public:
    ThreadUtils() = delete;
    static bool SetThreadName(std::thread&, const std::wstring&);
};
}
}
