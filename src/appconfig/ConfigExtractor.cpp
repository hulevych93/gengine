#include "ConfigExtractor.h"

#include <memory>
#include <cstring>
#include <boost/format.hpp>
#include <boost/filesystem/fstream.hpp>

#include <core/Encoding.h>
#include <filesystem/Filesystem.h>

extern void* g_module_instance;

namespace fs = boost::filesystem;

namespace Gengine {
namespace AppConfig {

ConfigExtractor::ConfigExtractor()
    : m_pos(0)
{
    m_stream = fs::ifstream(Filesystem::GetModuleFilePath(g_module_instance), std::ios::in | std::ios::binary);
    if (!m_stream)
    {
        throw std::exception();
    }
}

bool ConfigExtractor::Extract(const std::wstring& to)
{
    auto config = FindConfig();
    if (config.first != 0 && config.second != 0)
    {
        return Save(config.first, config.second, to);
    }
    return false;
}

bool ConfigExtractor::Extract(std::string& to)
{
    auto config = FindConfig();
    if (config.first != 0 && config.second != 0)
    {
        to = Read(config.first, config.second);
        return !to.empty();
    }
    return false;
}

bool ConfigExtractor::Save(size_t offset, size_t size, const std::wstring& path) const
{
    fs::ofstream output(path, std::ios::out | std::ios::binary);
    auto buffer = Read(offset, size);
    if (!buffer.empty())
    {
        output << buffer;
        return true;
    }

    return false;
}

std::string ConfigExtractor::Read(size_t offset, size_t size) const
{
    std::string out;
    auto buffer = std::make_unique<char[]>(ReadSize);

    size_t readSize = std::min(ReadSize, size);
    size_t bytesWritten = 0;

    // We may have been at the end of a stream before, having read less bytes then asked
    m_stream.clear();
    m_stream.seekg(offset, std::ios::beg);

    for (; !m_stream.eof() && bytesWritten < size;)
    {
        m_stream.read(buffer.get(), readSize);

        if (!m_stream.fail())
        {
            out += std::string(buffer.get(), m_stream.gcount());

            bytesWritten += m_stream.gcount();
            readSize = std::min(ReadSize, size - bytesWritten);
        }
    }

    return out;
}

std::pair<size_t, size_t> ConfigExtractor::FindConfig() const
{
    auto moduleName = toUtf8(Filesystem::GetModuleName(g_module_instance));
    auto marker = boost::str(boost::format(ConfigDelemiterStart) % moduleName);
    auto startPos = Find(marker);
    if (startPos != std::string::npos)
    {
        startPos += marker.size();
        auto configSize = Find(boost::str(boost::format(ConfigDelemiterEnd) % moduleName));
        if (configSize != std::string::npos)
        {
            configSize -= startPos;
            return std::make_pair(startPos, configSize);
        }
    }
    return std::pair<size_t, size_t>();
}

size_t ConfigExtractor::Find(const std::string& pattern) const
{
    auto offset = pattern.size() - 1;
    auto patternPosition = m_buffer.find(pattern);
    if (patternPosition != std::string::npos)
    {
        return m_pos - offset + patternPosition;
    }
    else
        m_buffer.resize(ReadSize);

    for (m_pos = m_stream.tellg(); m_pos != -1 && !m_stream.eof(); m_pos += m_stream.gcount())
    {
        m_stream.read(const_cast<char*>(m_buffer.c_str()) + offset, ReadSize - offset);
        m_buffer.resize(m_stream.gcount() + offset);

        auto patternPosition = m_buffer.find(pattern);
        if (patternPosition != std::string::npos)
        {
            return m_pos - offset + patternPosition;
        }

        memcpy(const_cast<char*>(m_buffer.c_str()), m_buffer.c_str() + ReadSize - offset, offset);
    }

    return std::string::npos;
}

const size_t ConfigExtractor::ReadSize = 128;
const std::string ConfigExtractor::ConfigDelemiterStart("#@-----%1%_CONFIG_START__-----@#");
const std::string ConfigExtractor::ConfigDelemiterEnd("#@-----%1%_CONFIG_END__-----@#");

}
}
