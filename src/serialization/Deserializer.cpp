
#include "Deserializer.h"
#include <core/Encoding.h>
#include <cstring>
#include <stdexcept>
#include "ISerializable.h"

namespace Gengine {
namespace Serialization {

Deserializer::Deserializer(const Blob& blob)
    : Deserializer(blob.GetData(), blob.GetSize()) {}

Deserializer::Deserializer(const void* data, size_t size)
    : m_base(static_cast<const std::uint8_t* const>(data)),
      m_data(const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(data))),
      m_size(size) {
  if (!m_data || !m_size) {
    throw std::invalid_argument("invalid_argument");
  }
}

Deserializer::Deserializer(const std::shared_ptr<Blob>& blob)
    : m_base(static_cast<const std::uint8_t* const>(blob->GetData())),
      m_data(static_cast<std::uint8_t*>(blob->GetData())),
      m_size(blob->GetSize()) {
  if (!m_data || !m_size) {
    throw std::invalid_argument("invalid_argument");
  }
}

size_t Deserializer::Used() const {
  return m_data - m_base;
}

bool Deserializer::PeekFixed(void* peek, size_t size) const {
  if (Used() + size <= m_size) {
    memcpy(peek, m_data, size);
    return true;
  } else {
    throw std::out_of_range("out_of_range");
  }
}

bool Deserializer::GetFixed(void* get, size_t size) const {
  if (Used() + size <= m_size) {
    memcpy(get, m_data, size);
    m_data += size;
    return true;
  } else {
    throw std::out_of_range("out_of_range");
  }
}

bool Deserializer::operator>>(std::uint8_t& data) const {
  return GetFixed(&data, sizeof(data));
}

bool Deserializer::operator>>(std::uint16_t& data) const {
  return GetFixed(&data, sizeof(data));
}

bool Deserializer::operator>>(std::uint32_t& data) const {
  return GetFixed(&data, sizeof(data));
}

bool Deserializer::operator>>(std::uint64_t& data) const {
  return GetFixed(&data, sizeof(data));
}

bool Deserializer::operator>>(std::int8_t& data) const {
  return GetFixed(&data, sizeof(data));
}

bool Deserializer::operator>>(std::int16_t& data) const {
  return GetFixed(&data, sizeof(data));
}

bool Deserializer::operator>>(std::int32_t& data) const {
  return GetFixed(&data, sizeof(data));
}

bool Deserializer::operator>>(std::int64_t& data) const {
  return GetFixed(&data, sizeof(data));
}

bool Deserializer::operator>>(bool& data_) const {
  std::uint8_t data;
  operator>>(data);
  data_ = data ? true : false;
  return true;
}

bool Deserializer::GetSize(size_t& size) const {
  if (Used() + sizeof(TSize) <= m_size) {
    auto sizeRaw = *reinterpret_cast<const TSize*>(m_data);
    size = static_cast<size_t>(sizeRaw);
    m_data += sizeof(sizeRaw);
    return true;
  } else {
    throw std::out_of_range("out_of_range");
  }
}

bool Deserializer::GetSized(TGetter getter) const {
  size_t size;
  GetSize(size);

  if (Used() + size <= m_size) {
    getter(m_data, size);
    m_data += size;
    return true;
  } else {
    throw std::out_of_range("out_of_range");
  }
}

bool Deserializer::operator>>(Blob& blob) const {
  return GetSized(
      [&](const std::uint8_t* data, size_t size) { blob.SetData(data, size); });
}

bool Deserializer::operator>>(ISerializable& serializable) const {
  serializable.Deserialize(const_cast<Deserializer&>(*this));
  return true;
}

}  // namespace Serialization
}  // namespace Gengine
