#include "SelfExtractedFileConfigReader.h"

#include "ConfigExtractor.h"

#include <filesystem/Filesystem.h>

namespace Gengine {
namespace AppConfig {

SelfExtractedFileConfigReader::SelfExtractedFileConfigReader(const config& conf)
    : FileConfigReader(FilePath, conf)
{}

bool SelfExtractedFileConfigReader::Load()
{
    ConfigExtractor extractor;
    if (extractor.Extract(FilePath))
    {
        return FileConfigReader::Load();
    }
    return false;
}

const std::wstring SelfExtractedFileConfigReader::FilePath = Filesystem::CombinePath(Filesystem::GetTempDirPath(), Filesystem::GetRandomFileName(L"config_%d"));


}
}
