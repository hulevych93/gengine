#pragma once

#include <interprocess-communication/param/Parameter.h>

#include <json/JSON.h>
#include <serialization/ISerializable.h>

#include <memory>
#include <vector>

namespace Gengine {
class Blob;
namespace InterprocessCommunication {

/**
 * InputParameters is the buffer for interprocess-communication.
 * The data in memory is going simultaneously.
 * /-----------------------------------------------/
 * / header / data / header / data / header / data /
 * / int8   / 23   / string / "ab" / bool   / true /
 * /-----------------------------------------------/
 */
class InputParameters final {
 public:
  /**
   * @brief The default constructor.
   */
  InputParameters();

  /**
   * @brief The move constructor.
   */
  InputParameters(InputParameters&& that);

  /**
   * @brief The copy constructor is deleted.
   */
  InputParameters(const InputParameters&) = delete;

  /**
   * @brief The move operator.
   * @param that
   * @return *this
   */
  InputParameters& operator=(InputParameters&& that);

  /**
   * @brief Deserialize the data from the binary buffer and make an
   * InputParameters object.
   *
   * @param[in] the pointer to the buffer.
   * @param[in] the size to be deserialized.
   *
   * The data is copied into an internal buffer and then parsed.
   * @returns optional InputParameters object.
   */
  static InputParameters makeParameters(const void* data, std::uint32_t size);

  /**
   * @brief Get a parameter's type by index.
   *
   * @param[in] index of the parameter.
   *
   * @return ParametersType of the param.
   * @throw If the parameter with given index doesn't exist the error is thrown.
   */
  ParametersTypes GetParameterType(std::int8_t index) const;

  /**
   * @brief Get a parameter's count.
   * @return params count.
   */
  std::int8_t GetParametersCount() const;

  /**
   * @brief Get value functions.
   * @return params count.
   */
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

  template <class T, std::enable_if_t<std::is_class<T>::value, void*> = nullptr>
  bool Get(std::int8_t index, std::shared_ptr<T>& value) const;

  template <class T,
            std::enable_if_t<!std::is_class<T>::value, void*> = nullptr>
  bool Get(std::int8_t index, std::shared_ptr<T>& value) const;

  template <class T>
  bool Get(std::int8_t index, std::unique_ptr<T>& value) const;

  template <class T>
  bool Get(std::int8_t index, std::vector<T>& container) const;

  template <class T>
  bool Get(std::int8_t index, std::deque<T>& container) const;

  template <class T>
  bool Get(std::int8_t index, std::list<T>& container) const;

  template <class T>
  bool Get(std::int8_t index, std::set<T>& container) const;

  template <class T>
  bool Get(std::int8_t index, std::unordered_set<T>& container) const;

  template <class T, class V>
  bool Get(std::int8_t index, std::map<T, V>& container) const;

  template <class T, class V>
  bool Get(std::int8_t index, std::unordered_map<T, V>& container) const;

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
                    ParametersTypes containerType) const;

  template <class T>
  bool GetContainerSingle(std::int8_t index, T& container) const;

  template <class T>
  bool GetContainerPaired(std::int8_t index, T& container) const;

  template <class Type>
  bool UnsafeGet(std::int8_t index, Type& value) const;

  bool CheckInBounds(std::int8_t index) const noexcept;

  template <class T>
  bool Get(std::int8_t index, std::shared_ptr<T>& value, std::true_type) const;

  template <class T>
  bool Get(std::int8_t index, std::shared_ptr<T>& value, std::false_type) const;

 private:
  std::vector<ParameterHeader*> m_parameters;
  std::unique_ptr<std::uint8_t[]> m_buffer;
  std::uint32_t m_size;
};

template <class T, std::enable_if_t<std::is_class<T>::value, void*>>
bool InputParameters::Get(std::int8_t index, std::shared_ptr<T>& value) const {
  return Get(index, value, std::is_final<T>{});
}

template <class T, std::enable_if_t<!std::is_class<T>::value, void*>>
bool InputParameters::Get(std::int8_t index, std::shared_ptr<T>& value) const {
  return Get(index, value, std::true_type{});
}

template <class T>
bool InputParameters::Get(std::int8_t index,
                          std::shared_ptr<T>& value,
                          std::true_type) const {
  value = std::make_shared<T>();
  return Get(index, *value);
}

template <class T>
bool InputParameters::Get(std::int8_t index,
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
bool InputParameters::Get(std::int8_t index, std::unique_ptr<T>& value) const {
  value = std::make_unique<T>();
  return Get(index, *value);
}

template <class T>
bool InputParameters::Get(std::int8_t index, std::vector<T>& container) const {
  return GetContainerSingle(index, container);
}

template <class T>
bool InputParameters::Get(std::int8_t index, std::deque<T>& container) const {
  return GetContainerSingle(index, container);
}

template <class T>
bool InputParameters::Get(std::int8_t index, std::list<T>& container) const {
  return GetContainerSingle(index, container);
}

template <class T>
bool InputParameters::Get(std::int8_t index, std::set<T>& container) const {
  return GetContainerSingle(index, container);
}

template <class T>
bool InputParameters::Get(std::int8_t index,
                          std::unordered_set<T>& container) const {
  return GetContainerSingle(index, container);
}

template <class T, class V>
bool InputParameters::Get(std::int8_t index, std::map<T, V>& container) const {
  return GetContainerPaired(index, container);
}

template <class T, class V>
bool InputParameters::Get(std::int8_t index,
                          std::unordered_map<T, V>& container) const {
  return GetContainerPaired(index, container);
}

template <class T>
bool InputParameters::GetContainer(std::int8_t index,
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
bool InputParameters::GetContainerSingle(std::int8_t index,
                                         T& container) const {
  return GetContainer(index, container, ParametersTypes::Container);
}

template <class T>
bool InputParameters::GetContainerPaired(std::int8_t index,
                                         T& container) const {
  return GetContainer(index, container, ParametersTypes::Map);
}

template <class Type>
bool InputParameters::UnsafeGet(std::int8_t index, Type& value) const {
  if (CheckInBounds(index)) {
    return false;
  }

  ParameterHeader* header = m_parameters[index];
  auto buf = reinterpret_cast<::uint8_t*>(header);
  buf += sizeof(ParameterHeader);
  value = *reinterpret_cast<Type*>(buf);
  return true;
}

}  // namespace InterprocessCommunication
}  // namespace Gengine
