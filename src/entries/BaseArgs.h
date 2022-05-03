#pragma once

#include <boost/variant.hpp>
#include <cstdint>

namespace Gengine {
namespace Entries {
template <class ArgvT>
struct BaseArgs final {
  BaseArgs(std::int32_t argc = 0, ArgvT* argv[] = nullptr)
      : argc(argc), argv(argv) {}

  std::int32_t argc;
  ArgvT** argv;
};

using wargv_type = BaseArgs<wchar_t>;
using argv_type = BaseArgs<char>;

struct WinArgs final {
  WinArgs(void* instance, void* prevInstance, char* cmdLine, int cmdShow)
      : instance(instance),
        prevInstance(prevInstance),
        cmdLine(cmdLine),
        cmdShow(cmdShow) {}

  void* instance;
  void* prevInstance;
  char* cmdLine;
  int cmdShow;
};

#if defined(_WIN32)
using args_type = boost::variant<wargv_type, argv_type, WinArgs>;
#elif __linux__ || __APPLE__
using args_type = boost::variant<wargv_type, argv_type>;
#endif

}  // namespace Entries
}  // namespace Gengine
