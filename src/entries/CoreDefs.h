#pragma once

#include <cstdint>
#include <string>
#include <boost/variant.hpp>
#include <entries/BaseArgs.h>

#if defined (_WIN32)
#include "Windows/EntryToolsFactory.h"
#elif __linux__ || __APPLE__
#include "Unix/LinuxEntryToolsFactory.h"
#endif

namespace Gengine {
namespace Entries {

#if defined (_WIN32)

using args_type = boost::variant<wargv_type, argv_type, WinArgs>;

#define FACTORY std::make_unique<EntryToolsFactory>()

#define IMPLEMENT_ENTRY \
void* g_module_instance = nullptr;\
extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,\
    LPSTR lpCmdLine, int nShowCmd)\
{\
    g_module_instance = hInstance;\
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);\
    Main mainObject(FACTORY);\
    auto args = WinArgs(hInstance, hPrevInstance, lpCmdLine, nShowCmd);\
    auto result = mainObject.Run(args);\
    CoUninitialize();\
    return result;\
}

#define IMPLEMENT_ENTRY_NO_COM \
void* g_module_instance = nullptr;\
extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,\
    LPSTR lpCmdLine, int nShowCmd)\
{\
    g_module_instance = hInstance;\
    Main mainObject(FACTORY);\
    auto args = WinArgs(hInstance, hPrevInstance, lpCmdLine, nShowCmd);\
    auto result = mainObject.Run(args);\
    return result;\
}

#if defined(UNICODE)
#define IMPLEMENT_CONSOLE_ENTRY \
void* g_module_instance = nullptr;\
int wmain(int argc, wchar_t** argv)\
{\
    Main mainObject(FACTORY);\
    auto args = BaseArgs<wchar_t>(argc, argv);\
    return mainObject.Run(args);\
}

#else
#define IMPLEMENT_CONSOLE_ENTRY \
void* g_module_instance = nullptr;\
int main(int argc, char** argv) \
{\
    Main mainObject(FACTORY);\
    auto args = BaseArgs<char>(argc, argv);\
    return mainObject.Run(args);\
}
#endif

#elif __linux__ || __APPLE__

using args_type = boost::variant<wargv_type, argv_type>;

#define FACTORY std::make_unique<LinuxEntryToolsFactory>()

#define IMPLEMENT_ENTRY \
void* g_module_instance = nullptr;\
int main(int argc, char** argv)\
{\
    static const int SomeData = 10u;\
    g_module_instance = (void*)&SomeData; \
    Main mainObject(FACTORY);\
    auto args = BaseArgs<char>(argc, argv);\
    return mainObject.Run(args);\
}

#define IMPLEMENT_CONSOLE_ENTRY IMPLEMENT_ENTRY

#endif

}
}
