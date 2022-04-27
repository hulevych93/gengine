#pragma once

#include <string>
#include <fstream>

namespace Gengine {
namespace AppConfig {

class ConfigExtractor
{
public:
    ConfigExtractor();

    bool Extract(const std::wstring& to);
    bool Extract(std::string& to);

private:
    bool Save(size_t offset, size_t size, const std::wstring& path) const;
    std::string Read(size_t offset, size_t size) const;
    std::pair<size_t, size_t> FindConfig() const;
    size_t Find(const std::string& pattern) const;

private:
    mutable std::ifstream m_stream;
    mutable std::string m_buffer;
    mutable std::int32_t m_pos;

private:
    static const size_t ReadSize;
    static const std::string ConfigDelemiterStart;
    static const std::string ConfigDelemiterEnd;
};

}
}
