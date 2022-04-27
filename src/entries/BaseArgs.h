#pragma once

#include <cstdint>

namespace Gengine {
namespace Entries {
template <class ArgvT>
struct BaseArgs
{
    BaseArgs(std::int32_t argc = 0, ArgvT* argv[] = nullptr)
        : argc(argc)
        , argv(argv)
    {}

    std::int32_t argc;
    ArgvT** argv;
};

using wargv_type = BaseArgs<wchar_t>;
using argv_type = BaseArgs<char>;

struct WinArgs
{
    WinArgs(void* instance, void* prevInstance, char* cmdLine, int cmdShow)
        : instance(instance)
        , prevInstance(prevInstance)
        , cmdLine(cmdLine)
        , cmdShow(cmdShow)
    {}

    void* instance;
    void* prevInstance;
    char* cmdLine;
    int cmdShow;
};
}
}
