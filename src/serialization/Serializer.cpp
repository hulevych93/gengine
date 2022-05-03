
#include "Serializer.h"
#include <core/Encoding.h>
#include <cstring>
#include "ISerializable.h"

namespace Gengine {
namespace Serialization {

Serializer::Serializer() : m_blob(std::make_shared<Blob>(4096)) {
  m_base = m_blob->GetData();
  m_data = m_blob->GetData();
  m_size = m_blob->GetSize();
}

Serializer::Serializer(size_t size) : m_blob(std::make_shared<Blob>(size)) {
  m_base = m_blob->GetData();
  m_data = m_blob->GetData();
  m_size = m_blob->GetSize();
}

std::shared_ptr<Blob> Serializer::GetBlob() {
  m_blob->Resize(Used());
  return m_blob;
}

void Serializer::Resize(size_t size) {
  auto used = Used();
  m_blob->Resize(size * 2, true);
  m_base = m_blob->GetData();
  m_data = m_blob->GetData() + used;
  m_size = m_blob->GetSize();
}

size_t Serializer::Used() const {
  return m_data - m_base;
}

bool Serializer::AddFixed(const void* data, size_t size) {
  if (Used() + size > m_size) {
    Resize(Used() + size);
  }

  memcpy(m_data, data, size);
  m_data += size;
  return true;
}

bool Serializer::operator<<(std::uint8_t data) {
  return AddFixed(&data, sizeof(data));
}

bool Serializer::operator<<(std::uint16_t data) {
  return AddFixed(&data, sizeof(data));
}

bool Serializer::operator<<(std::uint32_t data) {
  return AddFixed(&data, sizeof(data));
}

bool Serializer::operator<<(std::uint64_t data) {
  return AddFixed(&data, sizeof(data));
}

bool Serializer::operator<<(std::int8_t data) {
  return AddFixed(&data, sizeof(data));
}

bool Serializer::operator<<(std::int16_t data) {
  return AddFixed(&data, sizeof(data));
}

bool Serializer::operator<<(std::int32_t data) {
  return AddFixed(&data, sizeof(data));
}

bool Serializer::operator<<(std::int64_t data) {
  return AddFixed(&data, sizeof(data));
}

bool Serializer::operator<<(bool data_) {
  std::uint8_t data = data_ ? 1 : 0;
  return operator<<(data);
}

bool Serializer::AddSize(size_t size_) {
  auto size = static_cast<TSize>(size_);
  return AddFixed(&size, sizeof(size));
}

bool Serializer::AddSized(const void* data, size_t size) {
  bool result = AddSize(size);
  return result & AddFixed(data, size);
}

bool Serializer::operator<<(const Blob& blob) {
  return AddSized(blob.GetData(), blob.GetSize());
}

bool Serializer::operator<<(const ISerializable& serializable) {
  return serializable.Serialize(*this);
}

}  // namespace Serialization
}  // namespace Gengine
