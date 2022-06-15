#pragma once

#include <core/Hashable.h>
#include <string>
#include <tuple>

namespace Gengine {
namespace InterprocessSynchronization {

struct executable_params {
  bool operator==(const executable_params& executable) const {
    return std::tie(path, params) ==
           std::tie(executable.path, executable.params);
  }
  std::wstring path;
  std::wstring params;
};

}  // namespace InterprocessSynchronization
}  // namespace Gengine

MAKE_HASHABLE(Gengine::InterprocessSynchronization::executable_params,
              t.path,
              t.params)