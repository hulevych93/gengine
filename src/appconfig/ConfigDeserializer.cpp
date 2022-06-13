#include "ConfigDeserializer.h"

namespace Gengine {
using namespace JSON;
using namespace Serialization;

namespace AppConfig {

ConfigDeserilizer::ConfigDeserilizer(const std::string& buffer)
    : buffer(buffer) {}

bool ConfigDeserilizer::operator()(JSON::IJsonSerializable& config) const {
  Value json;
  json.Deserialize(buffer);
  OutputValue output(json);
  return output >> config;
}

bool ConfigDeserilizer::operator()(Serialization::ISerializable& config) const {
  Deserializer output{buffer.data(), buffer.size()};
  return output >> config;
}

}  // namespace AppConfig
}  // namespace Gengine
