#pragma once

#include <memory>
#include <string>

namespace Gengine {

/**
 * @brief The Blob class (e.g. BLOB)
 *
 * A BLOB (binary large object) is a varying-length binary string that can be up
 * to 2,147,483,647 characters long. Like other binary * types, BLOB strings are
 * not associated with a code page. In addition, BLOB strings do not hold
 * character data.
 */
class Blob final {
 public:
  /**
   * @brief Makes blob of zero size.
   */
  Blob();

  /**
   * The allocated space is filled with zeros.
   * @param[in] size to be allocated in blob.
   */
  explicit Blob(size_t size);

  /**
   * The constructor makes a copy of the provided data.
   * @param[in] data to be stored in blob.
   * @param[in] size of data to be stored.
   */
  Blob(const void* data, size_t size);

  /**
   * @brief Copy constructor.
   */
  Blob(const Blob& that);

  /**
   * @brief Move constructor.
   */
  Blob(Blob&& that);

  /**
   * @brief Copy assignment operator.
   */
  Blob& operator=(const Blob& that);

  /**
   * @brief Move assignment operator.
   */
  Blob& operator=(Blob&& that);

  /**
   * @brief Equal comparison operator.
   */
  bool operator==(const Blob& that) const;

  /**
   * @brief Not equal comparison operator.
   */
  bool operator!=(const Blob& that) const;

  /**
   * @brief Return a pointer to internal data buffer.
   */
  std::uint8_t* GetData();

  /**
   * @brief Return a const pointer to internal data buffer.
   */
  const std::uint8_t* GetData() const;

  /**
   * @brief Return a size of the internal data buffer.
   */
  size_t GetSize() const;

  /**
   * @brief Replace the content of the blob with given data.
   *
   * @param[in] data to be set. The data is copied into internal buffer.
   * @param[in] size of the data buffer.
   */
  void SetData(const void* data, size_t size);

  /**
   * @brief Append new data to the content of the blob.
   *
   * @param[in] data to be append. The data is copied into internal buffer.
   * @param[in] size of the data buffer.
   */
  void AddData(const void* data, size_t size);

  /**
   * @brief Resize an internal buffer.
   *
   * @param[in] new size. If the size is lower than current the actual buffer is
   * not changed.
   * @param[in] preserve the data stored in buffer during reallocating the
   * buffer.
   */
  void Resize(size_t size, bool preserve = true);

  /**
   * @brief Trim an internal buffer.
   *
   * Trims current buffer to a new one buffer using begin/end positions in the
   * current one. The size of new buffer is litterally equal to end - begin.
   *
   * @param[in] begin index in the current buffer to be preserved in new one.
   * @param[in] end index in the current buffer to be preserved in new one. Pass
   * -1 if the current end is ok.
   */
  void TrimTo(size_t begin, size_t end = -1);

  /**
   * @brief Clear an internal buffer.
   *
   */
  void Clear();

  std::string ToString() const;

 private:
  std::unique_ptr<std::uint8_t[]> m_data;
  size_t m_size;
};
}  // namespace Gengine
