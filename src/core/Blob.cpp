#include <core/Blob.h>
#include <cstring>

namespace Gengine {
Blob::Blob() : m_size(0) {}

Blob::Blob(size_t size)
    : m_data(std::make_unique<std::uint8_t[]>(size)), m_size(size) {
  memset(m_data.get(), 0, size);
}

Blob::Blob(const void* data, size_t size)
    : m_data(std::make_unique<std::uint8_t[]>(size)), m_size(size) {
  memcpy(m_data.get(), data, m_size);
}

Blob::Blob(const Blob& that)
    : m_data(std::make_unique<std::uint8_t[]>(that.m_size)),
      m_size(that.m_size) {
  memcpy(m_data.get(), that.m_data.get(), m_size);
}

Blob::Blob(Blob&& that) : m_data(std::move(that.m_data)), m_size(that.m_size) {
  that.m_size = 0;
}

Blob& Blob::operator=(const Blob& that) {
  if (this == &that) {
    return *this;
  }
  SetData(that.m_data.get(), that.m_size);
  return *this;
}

Blob& Blob::operator=(Blob&& that) {
  if (this == &that) {
    return *this;
  }
  m_data = std::move(that.m_data);
  m_size = that.m_size;
  that.m_size = 0;
  return *this;
}

bool Blob::operator==(const Blob& that) const {
  if (this == &that) {
    return true;
  }
  if (m_size == that.m_size) {
    return memcmp(m_data.get(), that.m_data.get(), m_size) == 0;
  } else {
    return false;
  }
}

bool Blob::operator!=(const Blob& that) const {
  return !operator==(that);
}

std::uint8_t* Blob::GetData() {
  return m_data.get();
}

const std::uint8_t* Blob::GetData() const {
  return m_data.get();
}

size_t Blob::GetSize() const {
  return m_size;
}

void Blob::SetData(const void* data, size_t size) {
  Resize(size, false);
  memcpy(m_data.get(), data, m_size);
}

void Blob::AddData(const void* data, size_t size) {
  auto prevSize = m_size;
  Resize(prevSize + size, true);
  memcpy(m_data.get() + prevSize, data, size);
}

void Blob::Resize(size_t size, bool preserve) {
  if (size > m_size) {
    if (preserve) {
      auto prevData = std::move(m_data);
      auto prevSize = m_size;
      m_data = std::make_unique<std::uint8_t[]>(size);
      m_size = size;
      memcpy(m_data.get(), prevData.get(), prevSize);
    } else {
      m_data = std::make_unique<std::uint8_t[]>(size);
      m_size = size;
    }
  } else {
    m_size = size;
  }
}

void Blob::TrimTo(size_t begin, size_t end /*= -1*/) {
  auto prevData = std::move(m_data);
  auto prevSize = m_size;
  m_size = end != -1 ? end - begin : prevSize - begin;
  m_data = std::make_unique<std::uint8_t[]>(m_size);
  memcpy(m_data.get(), prevData.get() + begin, m_size);
}

void Blob::Clear() {
  m_data.reset();
  m_size = 0;
}

std::string Blob::ToString() const {
  return "BLOB";
}

}  // namespace Gengine
