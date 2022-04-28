#include <entries/TestEntry.h>

#include <entries/Main.h>
#include <entries/EntryRegistry.h>

#include <core/Logger.h>

#if defined(BUILD_WINDOWS)
#include <Windows.h>
#endif


using namespace Gengine;
using namespace Entries;

REGISTER_TESTS_ENTRY(GTestModule)
IMPLEMENT_CONSOLE_ENTRY
