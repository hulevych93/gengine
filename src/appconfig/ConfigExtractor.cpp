#include "ConfigExtractor.h"

#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <cstring>
#include <memory>

#include <core/Encoding.h>

namespace fs = boost::filesystem;

namespace Gengine {
namespace AppConfig {

ConfigExtractor::ConfigExtractor(std::ifstream&& stream,
                                 const std::string& module)
    : m_stream(std::move(stream)), m_module(module), m_pos(0) {}

ConfigExtractor ConfigExtractor::makeExtractor(const std::string& file,
                                               const std::string& module) {
  auto stream = std::ifstream(file, std::ios::in | std::ios::binary);
  if (!stream) {
    throw std::runtime_error("can't create file stream");
  }
  return ConfigExtractor{std::move(stream), module};
}

bool ConfigExtractor::Extract(const std::string& filePath) {
  auto config = FindConfig();
  if (config.first != 0 && config.second != 0) {
    return Save(config.first, config.second, filePath);
  }
  return false;
}

bool ConfigExtractor::Extract(std::string& to) {
  auto config = FindConfig();
  if (config.first != 0 && config.second != 0) {
    to = Read(config.first, config.second);
    return !to.empty();
  }
  return false;
}

bool ConfigExtractor::Save(size_t offset,
                           size_t size,
                           const std::string& path) const {
  fs::ofstream output(path, std::ios::out | std::ios::binary);
  auto buffer = Read(offset, size);
  if (!buffer.empty()) {
    output << buffer;
    return true;
  }

  return false;
}

std::string ConfigExtractor::Read(size_t offset, size_t size) const {
  std::string out;
  auto buffer = std::make_unique<char[]>(ReadSize);

  size_t readSize = std::min(ReadSize, size);
  size_t bytesWritten = 0;

  // We may have been at the end of a stream before, having read less bytes then
  // asked
  m_stream.clear();
  m_stream.seekg(offset, std::ios::beg);

  for (; !m_stream.eof() && bytesWritten < size;) {
    m_stream.read(buffer.get(), readSize);

    if (!m_stream.fail()) {
      out += std::string(buffer.get(), m_stream.gcount());

      bytesWritten += m_stream.gcount();
      readSize = std::min(ReadSize, size - bytesWritten);
    }
  }

  return out;
}

std::pair<size_t, size_t> ConfigExtractor::FindConfig() const {
  auto marker = boost::str(boost::format(ConfigDelemiterStart) % m_module);
  auto startPos = Find(marker);
  if (startPos != std::string::npos) {
    startPos += marker.size();
    auto configSize =
        Find(boost::str(boost::format(ConfigDelemiterEnd) % m_module));
    if (configSize != std::string::npos) {
      configSize -= startPos;
      return std::make_pair(startPos, configSize);
    }
  }
  return std::pair<size_t, size_t>();
}

size_t ConfigExtractor::Find(const std::string& pattern) const {
  const auto offset = pattern.size() - 1;
  auto search = [&]() {
    auto patternPosition = m_buffer.find(pattern);
    if (patternPosition != std::string::npos) {
      return m_pos - offset + patternPosition;
    }
    return std::string::npos;
  };

  auto foundPos = search();
  if (foundPos != std::string::npos) {
    return foundPos;
  } else
    m_buffer.resize(ReadSize);

  for (m_pos = m_stream.tellg(); m_pos != -1 && !m_stream.eof();
       m_pos += m_stream.gcount()) {
    m_stream.read(const_cast<char*>(m_buffer.c_str()) + offset,
                  ReadSize - offset);
    m_buffer.resize(m_stream.gcount() + offset);

    foundPos = search();
    if (foundPos != std::string::npos) {
      return foundPos;
    }

    memcpy(const_cast<char*>(m_buffer.c_str()),
           m_buffer.c_str() + ReadSize - offset, offset);
  }

  return search();
}

const size_t ConfigExtractor::ReadSize = 128;
const std::string ConfigExtractor::ConfigDelemiterStart(
    "#@-----%1%_CONFIG_START__-----@#");
const std::string ConfigExtractor::ConfigDelemiterEnd(
    "#@-----%1%_CONFIG_END__-----@#");

}  // namespace AppConfig
}  // namespace Gengine
