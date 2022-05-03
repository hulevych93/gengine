#pragma once

#include <serialization/Common.h>

namespace Gengine {
namespace Serialization {

class ISerializable;

class Serializer : public boost::static_visitor<bool> {
 public:
  typedef std::uint32_t TSize;

 public:
  Serializer();
  Serializer(size_t size);

  std::shared_ptr<Blob> GetBlob();

 public:
  bool operator<<(std::uint8_t data);
  bool operator<<(std::uint16_t data);
  bool operator<<(std::uint32_t data);
  bool operator<<(std::uint64_t data);
  bool operator<<(std::int8_t data);
  bool operator<<(std::int16_t data);
  bool operator<<(std::int32_t data);
  bool operator<<(std::int64_t data);
  bool operator<<(bool data);
  bool operator<<(const Blob& blob);
  bool operator<<(const ISerializable& serializable);

  template <class T>
  typename std::enable_if<std::is_enum<T>::value, bool>::type operator<<(
      const T& data) {
    return AddFixed(&data, sizeof(data));
  }

  template <class _Elem, class _Traits, class _Alloc>
  bool operator<<(const std::basic_string<_Elem, _Traits, _Alloc>& string) {
    return AddSized(string.data(), string.size() * sizeof(_Elem));
  }

  template <class T>
  bool operator<<(const std::shared_ptr<T>& object) {
    bool result = operator<<(object != nullptr ? static_cast<std::int8_t>(1)
                                               : static_cast<std::int8_t>(0));
    if (object)
      result &= operator<<(*object);
    return result;
  }

  template <class T>
  bool operator<<(const std::unique_ptr<T>& object) {
    bool result = operator<<(object != nullptr ? static_cast<std::int8_t>(1)
                                               : static_cast<std::int8_t>(0));
    if (object)
      result &= operator<<(*object);
    return result;
  }

  template <class T>
  bool operator<<(const boost::optional<T>& object) {
    bool result = operator<<(object ? static_cast<std::int8_t>(1)
                                    : static_cast<std::int8_t>(0));
    if (object)
      result &= operator<<(object.get());
    return result;
  }

  template <class... T>
  bool operator<<(const boost::variant<T...>& object) {
    if (operator<<(object.which())) {
      return boost::apply_visitor(*this, object);
    }
    return false;
  }

  template <class T>
  bool operator()(const T& operand) {
    return operator<<(operand);
  }

  template <class T>
  bool operator<<(const std::vector<T>& container) {
    return AddContainer(container);
  }

  template <class T>
  bool operator<<(const std::deque<T>& container) {
    return AddContainer(container);
  }

  template <class T>
  bool operator<<(const std::list<T>& container) {
    return AddContainer(container);
  }

  template <class T, class P, class A>
  bool operator<<(const std::set<T, P, A>& container) {
    return AddContainer(container);
  }

  template <class T>
  bool operator<<(const std::unordered_set<T>& container) {
    return AddContainer(container);
  }

  template <class K, class V>
  bool operator<<(const std::map<K, V>& container) {
    return AddContainer(container);
  }

  template <class K, class V>
  bool operator<<(const std::unordered_map<K, V>& container) {
    return AddContainer(container);
  }

  template <class K, class V>
  bool operator<<(const std::pair<K, V>& pair) {
    bool result = operator<<(pair.first);
    result &= operator<<(pair.second);
    return result;
  }

 protected:
  bool AddFixed(const void* data, size_t size);
  bool AddSized(const void* data, size_t size);
  bool AddSize(size_t size);

  size_t Used() const;
  void Resize(size_t size);

  template <class T>
  bool AddContainer(const T& container) {
    bool result = AddSize(container.size());
    for (const auto& value : container) {
      result &= operator<<(value);
    }
    return result;
  }

 private:
  std::shared_ptr<Blob> m_blob;
  const std::uint8_t* m_base;
  std::uint8_t* m_data;
  size_t m_size;
};

}  // namespace Serialization
}  // namespace Gengine
