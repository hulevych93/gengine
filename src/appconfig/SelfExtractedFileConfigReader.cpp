#include "SelfExtractedFileConfigReader.h"

#include "ConfigExtractor.h"

#include <core/Encoding.h>
#include <filesystem/Filesystem.h>

extern void* g_module_instance;

namespace Gengine {
namespace AppConfig {

SelfExtractedFileConfigReader::SelfExtractedFileConfigReader(const config& conf)
    : FileConfigReader(FilePath, conf) {}

bool SelfExtractedFileConfigReader::Load() {
  const auto moduleFilePath = Filesystem::GetModuleFilePath(g_module_instance);
  const auto moduleName =
      Filesystem::GetFileNameWithoutExtension(moduleFilePath);

  auto extractor = ConfigExtractor::makeExtractor(toUtf8(moduleFilePath),
                                                  toUtf8(moduleName));
  if (extractor.Extract(toUtf8(FilePath))) {
    return FileConfigReader::Load();
  }
  return false;
}

const std::wstring SelfExtractedFileConfigReader::FilePath =
    Filesystem::CombinePath(Filesystem::GetTempDirPath(),
                            Filesystem::GetRandomFileName(L"config_%d"));

}  // namespace AppConfig
}  // namespace Gengine
