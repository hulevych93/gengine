#pragma once

#include <appconfig/ConfigCommonDefs.h>

namespace Gengine {
namespace AppConfig {

struct ConfigSerilizer : public boost::static_visitor<bool> {
  ConfigSerilizer(std::string& buffer);
  bool operator()(const JSON::IJsonSerializable& config) const;
  std::string& buffer;
};

}  // namespace AppConfig
}  // namespace Gengine
