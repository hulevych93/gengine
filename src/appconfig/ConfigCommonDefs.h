#pragma once

#include <boost/variant.hpp>
#include <json/JSON.h>

namespace Gengine {
namespace AppConfig {
using config = boost::variant<JSON::IJsonSerializable&>;
}
}

