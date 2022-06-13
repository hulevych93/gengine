#pragma once

#include <json/JSON.h>
#include <serialization/ISerializable.h>

#include <boost/variant.hpp>

namespace Gengine {
namespace AppConfig {
using config =
    boost::variant<JSON::IJsonSerializable&, Serialization::ISerializable&>;

template<typename ConfigReaderType, typename ConfigType, typename... Args>
ConfigReaderType makeJsonConfigReader(ConfigType& configuration, Args&&... args) {
    return ConfigReaderType{static_cast<JSON::IJsonSerializable&>(configuration), std::forward<Args>(args)...};
}

template<typename ConfigReaderType, typename ConfigType, typename... Args>
ConfigReaderType makeBinaryConfigReader(ConfigType& configuration, Args&&... args) {
    return ConfigReaderType{static_cast<Serialization::ISerializable&>(configuration), std::forward<Args>(args)...};
}

}
}  // namespace Gengine
