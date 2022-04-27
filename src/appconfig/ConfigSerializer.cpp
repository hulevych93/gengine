#include "ConfigSerializer.h"

namespace Gengine {
using namespace JSON;

namespace AppConfig {

ConfigSerilizer::ConfigSerilizer(std::string& buffer)
    : buffer(buffer)
{}
bool ConfigSerilizer::operator()(const JSON::IJsonSerializable& config) const
{
    Value value;
    InputValue input(value);
    auto result = input << config;
    value.Serialize(buffer);
    return result;
}

}
}


