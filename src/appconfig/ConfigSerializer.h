#pragma once

#include <appconfig/ConfigCommonDefs.h>

namespace Gengine {
namespace AppConfig {

struct ConfigSerilizer final : public boost::static_visitor<bool> {
  ConfigSerilizer(std::string& buffer);
  bool operator()(const JSON::IJsonSerializable& config) const;
  bool operator()(const Serialization::ISerializable& config) const;
  std::string& buffer;
};

}  // namespace AppConfig
}  // namespace Gengine
