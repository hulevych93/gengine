#pragma once

#include <appconfig/BufferConfigReader.h>

namespace Gengine {
namespace AppConfig {

class SelfExtractedBufferedConfigReader : public BufferConfigReader
{
public:
    SelfExtractedBufferedConfigReader(const config& conf);

    bool Load() override;
};

}
}
