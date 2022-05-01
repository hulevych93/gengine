#pragma once

#include <string>
#include <fstream>

namespace Gengine {
namespace AppConfig {

class ConfigExtractor final
{
public:
    static ConfigExtractor makeExtractor(const std::string& file, const std::string& module);

    bool Extract(const std::string& filePath);

    bool Extract(std::string& to);

private:
    explicit ConfigExtractor(std::ifstream&& stream, const std::string& module);

    bool Save(size_t offset, size_t size, const std::string& path) const;
    std::string Read(size_t offset, size_t size) const;
    std::pair<size_t, size_t> FindConfig() const;
    size_t Find(const std::string& pattern) const;

private:
    const std::string m_module;
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
