#pragma once

#include <tuple>
#include <string>
#include <core/Hashable.h>

namespace Gengine {
namespace InterprocessSynchronization {

struct executable_params
{
    bool operator==(const executable_params& executable) const
    {
        return std::tie(path, params)
            == std::tie(executable.path, executable.params);
    }
    std::wstring path;
    std::wstring params;
};

}
}

MAKE_HASHABLE(Gengine::InterprocessSynchronization::executable_params, t.path, t.params)