#include "SelExtractedBufferedConfigReader.h"

#include "ConfigExtractor.h"

namespace Gengine {
namespace AppConfig {

SelExtractedBufferedConfigReader::SelExtractedBufferedConfigReader(const config& conf)
    : BufferConfigReader(conf)
{}

bool SelExtractedBufferedConfigReader::Load()
{
    ConfigExtractor extractor;
    if (extractor.Extract(m_buffer))
    {
        return BufferConfigReader::Load();
    }
    return false;
}

}
}