#pragma once

#include <core/Blob.h>
#include <interprocess-communication/param/Parameter.h>

#include <json/JSON.h>
#include <serialization/ISerializable.h>
#include <string>

namespace Gengine {
namespace InterprocessCommunication {
struct ParameterHeader;

/**
 * @brief The OutputParameters class
 *
 * OutputParameters is the buffer for interprocess-communication.
 * The data in memory is going simultaneously.
 * /-----------------------------------------------/
 * / header / data / header / data / header / data /
 * / int8   / 23   / string / "ab" / bool   / true /
 * /-----------------------------------------------/
 */
class OutputParameters final {
 public:
  /**
   * @brief The default constructor.
   */
  OutputParameters() = default;

  /**
   * @brief The copy constructor is deleted.
   */
  OutputParameters(const OutputParameters&) = delete;

  /**
   * @brief Get buffer size.
   * @return buffer size;
   */
  std::uint32_t GetSize() const { return m_filedSize; }

  /**
   * @brief Get buffer raw ptr
   * @return buffer raw ptr
   */
  const void* GetData() const { return m_buffer.get(); }

  /**
   * @name Append functions.
   * @param[in] data to store into parametes.
   */
  ///@{
  void Append(bool value);
  void Append(void* value);
  void Append(std::int8_t value);
  void Append(std::int16_t value);
  void Append(std::int32_t value);
  void Append(std::int64_t value);
  void Append(std::uint8_t value);
  void Append(std::uint16_t value);
  void Append(std::uint32_t value);
  void Append(std::uint64_t value);
  void Append(const std::wstring& value);
  void Append(const std::string& value);
  void Append(const Blob& value);

  void Append(const Serialization::ISerializable& type);
  void Append(const JSON::IJsonSerializable& type);

  template <class T>
  void Append(const std::shared_ptr<T>& type);

  template <class T>
  void Append(const std::unique_ptr<T>& type);

  template <class T>
  void Append(const std::vector<T>& container);

  template <class T>
  void Append(const std::deque<T>& container);

  template <class T>
  void Append(const std::set<T>& container);

  template <class T>
  void Append(const std::unordered_set<T>& container);

  template <class T>
  void Append(const std::list<T>& container);

  template <class T, class V>
  void Append(const std::map<T, V>& container);

  template <class T, class V>
  void Append(const std::unordered_map<T, V>& container);
  ///@}

 private:
  template <class T>
  void AppendContainer(const T& container, ParametersTypes containerType);

  template <class T>
  void AppendSigleContainer(const T& container);

  template <class T>
  void AppendPairedContainer(const T& container);

  template <class Type>
  void AppendIntegerParameter(Type value, ParametersTypes type);

  void AppendSizedParameter(const void* data,
                            std::uint32_t size,
                            ParametersTypes type);
  void* AppendSizedParameter(std::uint32_t size, ParametersTypes type);

  void AllocateBufferSpace(std::uint32_t size,
                           ParameterHeader** header,
                           std::uint8_t** data);

 private:
  std::unique_ptr<std::uint8_t[]> m_buffer;
  std::uint32_t m_allocatedSize = 0u;
  std::uint32_t m_filedSize = 0u;
};

template <class T>
void OutputParameters::Append(const std::shared_ptr<T>& type) {
  if (!type) {
    throw std::runtime_error{"Append std::shared_ptr<T> null value."};
  }
  Append(*type);
}

template <class T>
void OutputParameters::Append(const std::unique_ptr<T>& type) {
  if (!type) {
    throw std::runtime_error{"Append std::unique_ptr<T> null value."};
  }
  Append(*type);
}

template <class T>
void OutputParameters::Append(const std::vector<T>& container) {
  AppendSigleContainer(container);
}

template <class T>
void OutputParameters::Append(const std::deque<T>& container) {
  AppendSigleContainer(container);
}

template <class T>
void OutputParameters::Append(const std::set<T>& container) {
  AppendSigleContainer(container);
}

template <class T>
void OutputParameters::Append(const std::unordered_set<T>& container) {
  AppendSigleContainer(container);
}

template <class T>
void OutputParameters::Append(const std::list<T>& container) {
  AppendSigleContainer(container);
}

template <class T, class V>
void OutputParameters::Append(const std::map<T, V>& container) {
  AppendPairedContainer(container);
}

template <class T, class V>
void OutputParameters::Append(const std::unordered_map<T, V>& container) {
  AppendPairedContainer(container);
}

template <class T>
void OutputParameters::AppendContainer(const T& container,
                                       ParametersTypes containerType) {
  Serialization::Serializer serializer;
  serializer << container;
  auto blob = serializer.GetBlob();
  auto data = reinterpret_cast<std::uint8_t*>(
      AppendSizedParameter(blob->GetSize(), containerType));
  memcpy(data, blob->GetData(), blob->GetSize());
}

template <class T>
void OutputParameters::AppendSigleContainer(const T& container) {
  AppendContainer(container, ParametersTypes::Container);
}

template <class T>
void OutputParameters::AppendPairedContainer(const T& container) {
  AppendContainer(container, ParametersTypes::Map);
}

}  // namespace InterprocessCommunication
}  // namespace Gengine
