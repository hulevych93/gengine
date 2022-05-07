#pragma once

#include <interprocess-communication/InterprocessCommonDefs.h>
#include <json/JSON.h>
#include <serialization/ISerializable.h>
#include <memory>
#include <vector>

namespace Gengine {
class Blob;
namespace InterprocessCommunication {
struct ParameterHeader;
struct polymorphic {};
class InputParameters final {
 public:
  InputParameters();
  InputParameters(const InputParameters&) = delete;

  bool Deserialize(void* data, std::uint32_t size);

  const ParameterHeader* GetParameterHeader(std::int8_t index) const;
  std::int8_t GetParametersCount() const;

  bool Get(std::int8_t index, bool& value) const;
  bool Get(std::int8_t index, void*& value) const;
  bool Get(std::int8_t index, std::uint8_t& value) const;
  bool Get(std::int8_t index, std::uint16_t& value) const;
  bool Get(std::int8_t index, std::uint32_t& value) const;
  bool Get(std::int8_t index, std::uint64_t& value) const;
  bool Get(std::int8_t index, std::int8_t& value) const;
  bool Get(std::int8_t index, std::int16_t& value) const;
  bool Get(std::int8_t index, std::int32_t& value) const;
  bool Get(std::int8_t index, std::int64_t& value) const;
  bool Get(std::int8_t index, std::wstring& value) const;
  bool Get(std::int8_t index, std::string& value) const;
  bool Get(std::int8_t index, Blob& value) const;

  bool Get(std::int8_t index, Serialization::ISerializable& value) const;
  bool Get(std::int8_t index, JSON::IJsonSerializable& value) const;

  template <class T>
  bool Get(std::int8_t index, std::shared_ptr<T>& value) const {
    return Get(index, value, std::is_final<T>{});
  }

  template <class T>
  bool Get(std::int8_t index, std::shared_ptr<T>& value, std::true_type) const {
    value = std::make_shared<T>();
    return Get(index, *value);
  }

  template <class T>
  bool Get(std::int8_t index,
           std::shared_ptr<T>& value,
           std::false_type) const {
    std::uint32_t type = 0;
    if (UnsafeGet(index, type)) {
      value = T::Create(type);
      return Get(index, *value);
    }
    return false;
  }

  template <class T>
  bool Get(std::int8_t index, std::unique_ptr<T>& value) const {
    value = std::make_unique<T>();
    return Get(index, *value);
  }

  template <class T>
  bool Get(std::int8_t index, std::vector<T>& container) const {
    return GetContainerSingle(index, container);
  }

  template <class T>
  bool Get(std::int8_t index, std::deque<T>& container) const {
    return GetContainerSingle(index, container);
  }

  template <class T>
  bool Get(std::int8_t index, std::list<T>& container) const {
    return GetContainerSingle(index, container);
  }

  template <class T>
  bool Get(std::int8_t index, std::set<T>& container) const {
    return GetContainerSingle(index, container);
  }

  template <class T>
  bool Get(std::int8_t index, std::unordered_set<T>& container) const {
    return GetContainerSingle(index, container);
  }

  template <class T, class V>
  bool Get(std::int8_t index, std::map<T, V>& container) const {
    return GetContainerPaired(index, container);
  }

  template <class T, class V>
  bool Get(std::int8_t index, std::unordered_map<T, V>& container) const {
    return GetContainerPaired(index, container);
  }

 private:
  template <class Type>
  bool Get(std::int8_t index, Type& value, ParametersTypes type) const;
  bool Get(std::int8_t index,
           void** data,
           std::uint32_t* size,
           ParametersTypes type) const;

  template <class T>
  bool GetContainer(std::int8_t index,
                    T& container,
                    ParametersTypes containerType) const {
    std::uint8_t* data = nullptr;
    std::uint32_t size = 0;
    if (Get(index, reinterpret_cast<void**>(&data), &size, containerType)) {
      Serialization::Deserializer deserializer(data, size);
      deserializer >> container;
      return true;
    }

    return false;
  }

  template <class T>
  bool GetContainerSingle(std::int8_t index, T& container) const {
    return GetContainer(index, container, ParametersTypes::Container);
  }

  template <class T>
  bool GetContainerPaired(std::int8_t index, T& container) const {
    return GetContainer(index, container, ParametersTypes::Map);
  }

  template <class Type>
  bool UnsafeGet(std::int8_t index, Type& value) const {
    if (index < 0 || index >= m_parameters.size()) {
      assert(0);
      return false;
    }
    ParameterHeader* header = m_parameters[index];
    auto buf = reinterpret_cast<::uint8_t*>(header);
    buf += sizeof(ParameterHeader);
    value = *reinterpret_cast<Type*>(buf);
    return true;
  }

 private:
  std::vector<ParameterHeader*> m_parameters;
  bool IsParameterHeaderValid(ParameterHeader* header);

  std::unique_ptr<std::uint8_t[]> m_buffer;
  std::uint32_t m_size;
};

}  // namespace InterprocessCommunication
}  // namespace Gengine
