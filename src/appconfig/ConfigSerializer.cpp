#include "ConfigSerializer.h"

namespace Gengine {
using namespace JSON;
using namespace Serialization;

namespace AppConfig {

ConfigSerilizer::ConfigSerilizer(std::string& buffer) : buffer(buffer) {}
bool ConfigSerilizer::operator()(const JSON::IJsonSerializable& config) const {
  Value value;
  InputValue input(value);
  const auto result = input << config;
  value.Serialize(buffer);
  return result;
}

bool ConfigSerilizer::operator()(
    const Serialization::ISerializable& config) const {
  Serializer value;
  const auto result = value << config;

  const auto blob = value.GetBlob();
  buffer = std::string{(char*)blob->GetData(), blob->GetSize()};
  return result;
}

}  // namespace AppConfig
}  // namespace Gengine
