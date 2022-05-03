#pragma once

#include <memory>
#include <string>

namespace Gengine {
class Blob final {
 public:
  Blob();
  explicit Blob(size_t size);
  Blob(const void* data, size_t size);

  Blob(const Blob& that);
  Blob(Blob&& that);

  Blob& operator=(const Blob& that);
  Blob& operator=(Blob&& that);

  bool operator==(const Blob& that) const;
  bool operator!=(const Blob& that) const;

  std::uint8_t* GetData();
  const std::uint8_t* GetData() const;
  size_t GetSize() const;

  // no realloc on smaller size
  void SetData(const void* data, size_t size);
  void AddData(const void* data, size_t size);
  void Resize(size_t size, bool preserve = true);
  void TrimTo(size_t begin, size_t end = -1);
  void Clear();

  std::string ToString() const;

 protected:
  std::unique_ptr<std::uint8_t[]> m_data;
  size_t m_size;
};
}  // namespace Gengine
