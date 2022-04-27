#pragma once

#include <appconfig/BufferConfigReader.h>

namespace Gengine {
namespace AppConfig {

class SelExtractedBufferedConfigReader : public BufferConfigReader
{
public:
    SelExtractedBufferedConfigReader(const config& conf);

    bool Load() override;
};

}
}