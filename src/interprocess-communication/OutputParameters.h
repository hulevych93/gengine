#pragma once

#include <core/Blob.h>
#include <interprocess-communication/InterprocessCommonDefs.h>
#include <json/JSON.h>
#include <serialization/ISerializable.h>
#include <string>

namespace Gengine {
namespace InterprocessCommunication {
struct ParameterHeader;
class OutputParameters final {
 public:
  OutputParameters();
  OutputParameters(const OutputParameters&) = delete;

  std::uint32_t GetSize() const { return m_filedSize; }

  const void* GetData() const { return m_buffer.get(); }

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
  void Append(const std::shared_ptr<T>& type) {
    Append(*type);
  }

  template <class T>
  void Append(const std::unique_ptr<T>& type) {
    Append(*type);
  }

  template <class T>
  void Append(const std::vector<T>& container) {
    AppendSigleContainer(container);
  }

  template <class T>
  void Append(const std::deque<T>& container) {
    AppendSigleContainer(container);
  }

  template <class T>
  void Append(const std::set<T>& container) {
    AppendSigleContainer(container);
  }

  template <class T>
  void Append(const std::unordered_set<T>& container) {
    AppendSigleContainer(container);
  }

  template <class T>
  void Append(const std::list<T>& container) {
    AppendSigleContainer(container);
  }

  template <class T, class V>
  void Append(const std::map<T, V>& container) {
    AppendPairedContainer(container);
  }

  template <class T, class V>
  void Append(const std::unordered_map<T, V>& container) {
    AppendPairedContainer(container);
  }

 private:
  template <class T>
  void AppendContainer(const T& container, ParametersTypes containerType) {
    Serialization::Serializer serializer;
    serializer << container;
    auto blob = serializer.GetBlob();
    auto data = reinterpret_cast<std::uint8_t*>(
        AppendSizedParameter(blob->GetSize(), containerType));
    memcpy(data, blob->GetData(), blob->GetSize());
  }

  template <class T>
  void AppendSigleContainer(const T& container) {
    AppendContainer(container, ParametersTypes::Container);
  }

  template <class T>
  void AppendPairedContainer(const T& container) {
    AppendContainer(container, ParametersTypes::Map);
  }

 private:
  template <class Type>
  void AppendIntegerParameter(Type value, ParametersTypes type);

  void AppendSizedParameter(const void* data,
                            std::uint32_t size,
                            ParametersTypes type);
  void* AppendSizedParameter(std::uint32_t size, ParametersTypes type);

 private:
  void AllocateBufferSpace(std::uint32_t size,
                           ParameterHeader** header,
                           std::uint8_t** data);

  std::unique_ptr<std::uint8_t[]> m_buffer;
  std::uint32_t m_allocatedSize;
  std::uint32_t m_filedSize;
};

}  // namespace InterprocessCommunication
}  // namespace Gengine
