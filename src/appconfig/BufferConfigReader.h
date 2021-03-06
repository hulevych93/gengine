#pragma once

#include <appconfig/ConfigCommonDefs.h>
#include <appconfig/IConfigReader.h>

namespace Gengine {
namespace AppConfig {

class BufferConfigReader : public IConfigReader {
 public:
  BufferConfigReader(const config& conf);
  BufferConfigReader(const config& conf, const std::string& buffer);

  bool Load() override;
  bool Save() const override;

  const std::string& GetBuffer() const;

 private:
  config m_config;

 protected:
  mutable std::string m_buffer;
};

}  // namespace AppConfig
}  // namespace Gengine
