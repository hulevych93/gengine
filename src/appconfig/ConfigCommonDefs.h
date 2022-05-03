#pragma once

#include <json/JSON.h>
#include <boost/variant.hpp>

namespace Gengine {
namespace AppConfig {
using config = boost::variant<JSON::IJsonSerializable&>;
}
}  // namespace Gengine
