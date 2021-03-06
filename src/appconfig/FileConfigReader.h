#pragma once

#include <appconfig/BufferConfigReader.h>
#include <appconfig/ConfigCommonDefs.h>

namespace Gengine {
namespace AppConfig {

class FileConfigReader : public BufferConfigReader {
 public:
  FileConfigReader(const config& conf, const std::wstring& path);

  bool Load() override;
  bool Save() const override;

 protected:
  std::wstring GetTemporaryPath() const;
  bool Swap() const;

 private:
  const std::wstring m_path;
};

}  // namespace AppConfig
}  // namespace Gengine
