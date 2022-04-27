#include "ConfigDeserializer.h"

namespace Gengine {
using namespace JSON;

namespace AppConfig {

ConfigDeserilizer::ConfigDeserilizer(const std::string& buffer)
    : buffer(buffer)
{}
bool ConfigDeserilizer::operator()(JSON::IJsonSerializable& config) const
{
    Value json;
    json.Deserialize(buffer);
    OutputValue output(json);
    return output >> config;
}

}
}
