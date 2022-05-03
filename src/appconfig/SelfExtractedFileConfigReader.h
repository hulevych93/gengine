#pragma once

#include <appconfig/FileConfigReader.h>

namespace Gengine {
namespace AppConfig {

class SelfExtractedFileConfigReader : public FileConfigReader {
 public:
  SelfExtractedFileConfigReader(const config& conf);

  bool Load() override;

 private:
  static const std::wstring FilePath;
};

}  // namespace AppConfig
}  // namespace Gengine
