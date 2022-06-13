#pragma once

#include <appconfig/ConfigCommonDefs.h>

namespace Gengine {
namespace AppConfig {

struct ConfigDeserilizer final : public boost::static_visitor<bool> {
  ConfigDeserilizer(const std::string& buffer);
  bool operator()(JSON::IJsonSerializable& config) const;
  bool operator()(Serialization::ISerializable& config) const;
  const std::string& buffer;
};

}  // namespace AppConfig
}  // namespace Gengine
