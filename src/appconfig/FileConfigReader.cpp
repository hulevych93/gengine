#include "FileConfigReader.h"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <core/Logger.h>

namespace fs = boost::filesystem;

namespace Gengine {
namespace AppConfig {
FileConfigReader::FileConfigReader(const std::wstring& path, const config& conf)
    : BufferConfigReader(conf)
    , m_path(path)
{}

bool FileConfigReader::Load()
{
    bool result = true;
    if(m_buffer.empty())
    {
        fs::ifstream file;
        file.open(m_path, std::ios::out);
        if (file.is_open())
        {
            auto line = std::string();
            while (std::getline(file, line))
            {
                m_buffer += line;
            }

            file.close();
        }

        if (!BufferConfigReader::Load())
        {
            GLOG_ERROR("Failed to load xml settings: %ls", m_path.c_str());
            result = false;
        }
    }

    return result;
}

bool FileConfigReader::Save() const
{
    bool result = true;
    fs::ofstream file;
    file.open(GetTemporaryPath());
    if (file.is_open())
    {
        if (BufferConfigReader::Save())
        {
            auto& storage = BufferConfigReader::GetBuffer();
            if (!storage.empty())
            {
                file << storage;
                if (!file.fail())
                {
                    file.close();
                    result = Swap();
                }
                else
                {
                    result = false;
                    GLOG_ERROR("Failed to write file: %ls", m_path);
                }
            }
        }
    }
    return result;
}

std::wstring FileConfigReader::GetTemporaryPath() const
{
    return m_path + L".tmp";
}

bool FileConfigReader::Swap() const
{
    boost::system::error_code ec;
    fs::rename(GetTemporaryPath(), m_path, ec);
    return ec.value() == 0;
}


}
}