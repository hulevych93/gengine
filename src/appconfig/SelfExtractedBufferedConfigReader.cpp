#include "SelfExtractedBufferedConfigReader.h"

#include "ConfigExtractor.h"

#include <core/Encoding.h>
#include <filesystem/Filesystem.h>

extern void* g_module_instance;

namespace Gengine {
namespace AppConfig {

SelfExtractedBufferedConfigReader::SelfExtractedBufferedConfigReader(
    const config& conf)
    : BufferConfigReader(conf) {}

bool SelfExtractedBufferedConfigReader::Load() {
  const auto moduleFilePath = Filesystem::GetModuleFilePath(g_module_instance);
  const auto moduleName =
      Filesystem::GetFileNameWithoutExtension(moduleFilePath);

  auto extractor = ConfigExtractor::makeExtractor(toUtf8(moduleFilePath),
                                                  toUtf8(moduleName));
  if (extractor.Extract(m_buffer)) {
    return BufferConfigReader::Load();
  }
  return false;
}

}  // namespace AppConfig
}  // namespace Gengine
