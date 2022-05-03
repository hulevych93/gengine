#include "BufferConfigReader.h"

#include "ConfigDeserializer.h"
#include "ConfigSerializer.h"

namespace Gengine {
namespace AppConfig {

BufferConfigReader::BufferConfigReader(const std::string& path,
                                       const config& conf)
    : m_buffer(path), m_config(conf) {}

BufferConfigReader::BufferConfigReader(const config& conf) : m_config(conf) {}

bool BufferConfigReader::Load() {
  return boost::apply_visitor(ConfigDeserilizer{m_buffer}, m_config);
}

bool BufferConfigReader::Save() const {
  return boost::apply_visitor(ConfigSerilizer{m_buffer}, m_config);
}

const std::string& BufferConfigReader::GetBuffer() const {
  return m_buffer;
}

}  // namespace AppConfig
}  // namespace Gengine
