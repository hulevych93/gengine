#pragma once

namespace Gengine {
namespace Diagnostic {
enum class DumpType
{
    Crash,
    ExitLock
};

enum class DumperType
{
    ProcessMemory,
    MemoryLeaks
};
}
}