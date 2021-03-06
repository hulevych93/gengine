#pragma once

#include <core/VariantCreator.h>
#include <serialization/Common.h>

namespace Gengine {
namespace Serialization {

class ISerializable;

/**
 * @brief The Deserializer class
 *
 * The class implements an output oriented binary buffer.
 * The buffer is sequenced and unsafe. The data should be get in order
 * of serialiation via Serializer class, in other case the behaviour is
 * undefined.
 */
class Deserializer final : public boost::static_visitor<bool> {
 private:
  using t_size = std::uint32_t;
  using t_getter = std::function<void(const std::uint8_t* data, size_t size)>;

 public:
  /**
   * @brief Deserializer
   * @param blob to get data from.
   *
   * The data from blob is not copied into the object.
   */
  Deserializer(const Blob& blob);

  /**
   * @brief Deserializer
   * @param data pointer to the buffer to get data from.
   * @param size of the buffer
   * @throw throws std::invalid_argument exception on nullptr buffer, etc.
   *
   * The data from blob is not copied into the object.
   */
  Deserializer(const void* data, size_t size);

  /**
   * @name operator>> functions
   * @param[out] data to store the get value.
   * @return true on success.
   * @throws std::out_of_range in case of the end of the buffer.
   */
  ///@{
  bool operator>>(char& data) const;
  bool operator>>(std::uint8_t& data) const;
  bool operator>>(std::uint16_t& data) const;
  bool operator>>(std::uint32_t& data) const;
  bool operator>>(std::uint64_t& data) const;
  bool operator>>(std::int8_t& data) const;
  bool operator>>(std::int16_t& data) const;
  bool operator>>(std::int32_t& data) const;
  bool operator>>(std::int64_t& data) const;
  bool operator>>(bool& data) const;
  bool operator>>(Blob& blob) const;
  bool operator>>(ISerializable& serializable) const;
  bool operator>>(boost::blank) const { return true; }

  template <class T>
  typename std::enable_if<std::is_enum<T>::value, bool>::type operator>>(
      T& data) const;

  template <class _Elem, class _Traits, class _Alloc>
  bool operator>>(std::basic_string<_Elem, _Traits, _Alloc>& string) const;

  template <class T>
  bool operator>>(std::unique_ptr<T>& object) const;

  template <class T>
  bool operator>>(std::shared_ptr<T>& value) const;

  template <class T>
  bool operator>>(boost::optional<T>& object) const;

  template <class... T>
  bool operator>>(boost::variant<T...>& object) const;

  template <class T>
  bool operator()(T& operand) const;

  template <class T>
  bool operator>>(std::vector<T>& container) const;

  template <class T>
  bool operator>>(std::deque<T>& container) const;

  template <class T>
  bool operator>>(std::list<T>& container) const;

  template <class T>
  bool operator>>(std::set<T>& container) const;

  template <class T>
  bool operator>>(std::unordered_set<T>& container) const;

  template <class K, class V>
  bool operator>>(std::map<K, V>& container) const;

  template <class T, class P, class A>
  bool operator>>(std::set<T, P, A>& container) const;

  template <class K, class V>
  bool operator>>(std::unordered_map<K, V>& container) const;

  template <class K, class V>
  bool operator>>(std::pair<K, V>& pair) const;
  ///@}

 private:
  bool PeekFixed(void* peek, size_t size) const;
  bool GetFixed(void* get, size_t size) const;
  bool GetSized(t_getter getter) const;
  bool GetSize(size_t& size) const;
  size_t Used() const;

 private:
  template <class T>
  bool GetContainerSingle(T& container) const;

  template <class T>
  bool GetContainerSingleSet(T& container) const;

  template <class T>
  bool GetContainerPaired(T& container) const;

  template <class T>
  bool GetShared(std::shared_ptr<T>& value, std::true_type) const;

  template <class T>
  bool GetShared(std::shared_ptr<T>& value, std::false_type) const;

 private:
  const std::uint8_t* const m_base;
  mutable std::uint8_t* m_data;
  const size_t m_size;
};

template <class T>
typename std::enable_if<std::is_enum<T>::value, bool>::type
Deserializer::operator>>(T& data) const {
  return GetFixed(&data, sizeof(data));
}

template <class _Elem, class _Traits, class _Alloc>
bool Deserializer::operator>>(
    std::basic_string<_Elem, _Traits, _Alloc>& string) const {
  return GetSized([&](const std::uint8_t* data, size_t size) {
    string.assign(reinterpret_cast<const _Elem*>(data), size / sizeof(_Elem));
  });
}

template <class T>
bool Deserializer::operator>>(std::unique_ptr<T>& object) const {
  std::int8_t res = {0};
  bool result = operator>>(res);
  if (res > 0) {
    object = std::make_unique<T>();
    result &= operator>>(*object);
  }
  return result;
}

template <class T>
bool Deserializer::operator>>(std::shared_ptr<T>& value) const {
  std::int8_t res = {0};
  bool result = operator>>(res);
  if (res > 0) {
    result &= GetShared(
        value, std::integral_constant < bool,
        std::is_final<T>::value || std::is_fundamental<T>::value > {});
  }
  return result;
}

template <class T>
bool Deserializer::operator>>(boost::optional<T>& object) const {
  std::int8_t res = {0};
  bool result = operator>>(res);
  if (res > 0) {
    T value;
    result &= operator>>(value);
    object = value;
  }
  return result;
}

template <class... T>
bool Deserializer::operator>>(boost::variant<T...>& object) const {
  std::int32_t which{0};
  if (operator>>(which)) {
    makeVariant(which, object);
    return boost::apply_visitor(*this, object);
  }
  return false;
}

template <class T>
bool Deserializer::operator()(T& operand) const {
  return operator>>(operand);
}

template <class T>
bool Deserializer::operator>>(std::vector<T>& container) const {
  return GetContainerSingle(container);
}

template <class T>
bool Deserializer::operator>>(std::deque<T>& container) const {
  return GetContainerSingle(container);
}

template <class T>
bool Deserializer::operator>>(std::list<T>& container) const {
  return GetContainerSingle(container);
}

template <class T>
bool Deserializer::operator>>(std::set<T>& container) const {
  return GetContainerSingleSet(container);
}

template <class T>
bool Deserializer::operator>>(std::unordered_set<T>& container) const {
  return GetContainerSingleSet(container);
}

template <class K, class V>
bool Deserializer::operator>>(std::map<K, V>& container) const {
  return GetContainerPaired(container);
}

template <class T, class P, class A>
bool Deserializer::operator>>(std::set<T, P, A>& container) const {
  return GetContainerSingleSet(container);
}

template <class K, class V>
bool Deserializer::operator>>(std::unordered_map<K, V>& container) const {
  return GetContainerPaired(container);
}

template <class K, class V>
bool Deserializer::operator>>(std::pair<K, V>& pair) const {
  bool result = operator>>(pair.first);
  result &= operator>>(pair.second);
  return result;
}

template <class T>
bool Deserializer::GetContainerSingle(T& container) const {
  size_t size;
  bool result = GetSize(size);

  for (size_t i = 0; i < size; i++) {
    typename T::value_type value;
    operator>>(value);
    container.emplace(container.end(), value);
  }
  return result;
}

template <class T>
bool Deserializer::GetContainerSingleSet(T& container) const {
  size_t size;
  bool result = GetSize(size);

  for (size_t i = 0; i < size; i++) {
    typename T::value_type value;
    operator>>(value);
    container.emplace(value);
  }
  return result;
}

template <class T>
bool Deserializer::GetContainerPaired(T& container) const {
  size_t size;
  bool result = GetSize(size);

  for (size_t i = 0; i < size; i++) {
    typename T::key_type key;
    typename T::mapped_type mapped;
    operator>>(key);
    operator>>(mapped);
    container.emplace(key, mapped);
  }
  return result;
}

template <class T>
bool Deserializer::GetShared(std::shared_ptr<T>& value, std::true_type) const {
  value = std::make_shared<T>();
  return operator>>(*value);
}

template <class T>
bool Deserializer::GetShared(std::shared_ptr<T>& value, std::false_type) const {
  std::uint32_t type = 0;
  PeekFixed(&type, sizeof(type));
  value = T::Create(type);
  return operator>>(*value);
}

}  // namespace Serialization
}  // namespace Gengine
