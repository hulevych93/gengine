#pragma once

#include <appconfig/ConfigCommonDefs.h>
#include <appconfig/BufferConfigReader.h>

namespace Gengine {
namespace AppConfig {

class FileConfigReader : public BufferConfigReader
{
public:
    FileConfigReader(const std::wstring& path, const config& conf);

    bool Load() override;
    bool Save() const override;

protected:
    std::wstring GetTemporaryPath() const;
    bool Swap() const;

private:
    const std::wstring m_path;
};

}
}