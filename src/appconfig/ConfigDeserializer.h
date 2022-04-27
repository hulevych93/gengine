#pragma once

#include <appconfig/ConfigCommonDefs.h>

namespace Gengine {
namespace AppConfig {

struct ConfigDeserilizer : public boost::static_visitor<bool>
{
    ConfigDeserilizer(const std::string& buffer);
    bool operator()(JSON::IJsonSerializable& config) const;
    const std::string& buffer;
};

}
}
