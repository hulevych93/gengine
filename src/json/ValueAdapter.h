#pragma once

#include <deque>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <json/Array.h>
#include <json/Common.h>
#include <json/Number.h>
#include <json/Object.h>

#include <core/VariantCreator.h>
#include <boost/optional.hpp>

namespace Gengine {
namespace JSON {

class InputValue final : public boost::static_visitor<bool> {
 public:
  InputValue(Value& value);

  bool operator<<(const IJsonSerializable& value);
  bool operator<<(const Object& value);
  bool operator<<(const Array& value);
  bool operator<<(const Number& value);
  bool operator<<(const Value& value);
  bool operator<<(const char_t* value);
  bool operator<<(const wstring_t& value);
  bool operator<<(const string_t& value);
  bool operator<<(real_t value);
  bool operator<<(uint64_t value);
  bool operator<<(int64_t value);
  bool operator<<(uint32_t value);
  bool operator<<(int32_t value);
  bool operator<<(uint16_t value);
  bool operator<<(int16_t value);
  bool operator<<(uint8_t value);
  bool operator<<(int8_t value);
  bool operator<<(bool value);
  bool operator<<(boost::blank value);

  template <class T>
  typename std::enable_if<std::is_enum<T>::value, bool>::type operator<<(
      const T& that) {
    return operator<<(static_cast<uint32_t>(that));
  }

  template <class T>
  bool operator<<(const std::shared_ptr<T>& object) {
    if (object) {
      return operator<<(*object);
    }
    return false;
  }

  template <class T>
  bool operator<<(const std::unique_ptr<T>& object) {
    if (object) {
      return operator<<(*object);
    }
    return false;
  }

  template <class T>
  bool operator<<(const boost::optional<T>& object) {
    if (object) {
      return operator<<(object.get());
    }
    return false;
  }

  template <class... T>
  bool operator<<(const boost::variant<T...>& object) {
    Object variantObject;
    variantObject[VariantKey] << object.which();

    Value variantData;
    InputValue input(variantData);

    auto result = boost::apply_visitor(input, object);

    variantObject[VariantData] << variantData;
    return result && operator<<(variantObject);
  }

  template <class T>
  bool operator()(const T& operand) {
    return operator<<(operand);
  }

  template <class T>
  bool operator<<(const std::vector<T>& container) {
    return AddContainerSingle(container);
  }

  template <class T>
  bool operator<<(const std::deque<T>& container) {
    return AddContainerSingle(container);
  }

  template <class T>
  bool operator<<(const std::list<T>& container) {
    return AddContainerSingle(container);
  }

  template <class T, class P, class A>
  bool operator<<(const std::set<T, P, A>& container) {
    return AddContainerSingle(container);
  }

  template <class T>
  bool operator<<(const std::unordered_set<T>& container) {
    return AddContainerSingle(container);
  }

  template <class K, class V>
  bool operator<<(const std::map<K, V>& container) {
    return AddContainerPaired(container);
  }

  template <class K, class V>
  bool operator<<(const std::unordered_map<K, V>& container) {
    return AddContainerPaired(container);
  }

  template <class K, class V>
  bool operator<<(const std::pair<K, V>& pair) {
    Object pairObject;
    pairObject[MapKey] << pair.first;
    pairObject[MapValue] << pair.second;
    operator<<(pairObject);
    return true;
  }

 protected:
  template <class T>
  bool AddContainerSingle(const T& container) {
    Array array;
    for (const auto& value : container) {
      Value value_;
      InputValue(value_) << value;
      array.push_back(value_);
    }
    if (array.size() > 0) {
      operator<<(array);
      return true;
    }
    return false;
  }

  template <class T>
  bool AddContainerPaired(const T& container) {
    Array array;
    for (const auto& value : container) {
      Object pairObject;
      pairObject[MapKey] << value.first;
      pairObject[MapValue] << value.second;
      array.push_back(pairObject);
    }
    if (array.size() > 0) {
      operator<<(array);
      return true;
    }
    return false;
  }

 private:
  Value& m_value;
};

class OutputValue final : public boost::static_visitor<bool> {
 public:
  OutputValue(const Value& value);

  bool operator>>(IJsonSerializable& value) const;
  bool operator>>(wstring_t& value) const;
  bool operator>>(string_t& value) const;
  bool operator>>(real_t& value) const;
  bool operator>>(uint64_t& value) const;
  bool operator>>(int64_t& value) const;
  bool operator>>(uint32_t& value) const;
  bool operator>>(int32_t& value) const;
  bool operator>>(uint16_t& value) const;
  bool operator>>(int16_t& value) const;
  bool operator>>(uint8_t& value) const;
  bool operator>>(int8_t& value) const;
  bool operator>>(Object& value) const;
  bool operator>>(Array& value) const;
  bool operator>>(bool& value) const;
  bool operator>>(boost::blank& value) const;

  template <class T>
  bool operator>>(std::shared_ptr<T>& data) const {
    T readed;
    if (operator>>(readed)) {
      data = std::make_shared<T>();
      *data = readed;
      return true;
    }
    return false;
  }

  template <class T>
  bool operator>>(std::unique_ptr<T>& data) const {
    T readed;
    if (operator>>(readed)) {
      data = std::make_unique<T>();
      *data = readed;
      return true;
    }
    return false;
  }

  template <class T>
  bool operator>>(boost::optional<T>& data) const {
    T readed;
    if (operator>>(readed)) {
      data = readed;
      return true;
    }
    return false;
  }

  template <class... T>
  bool operator>>(boost::variant<T...>& object) const {
    const Object variantObject;
    if (operator>>(const_cast<Object&>(variantObject))) {
      std::int32_t which{0};
      if (variantObject[VariantKey] >> which) {
        create_variant(which, object);
        const OutputValue output = variantObject[VariantData];
        return boost::apply_visitor(output, object);
      }
    }
    return false;
  }

  template <class T>
  bool operator()(T& operand) const {
    return operator>>(operand);
  }

  template <class T>
  typename std::enable_if<std::is_enum<T>::value, bool>::type operator>>(
      T& data) const {
    if (m_value.IsNumber()) {
      data = static_cast<T>(m_value.ToNumber().ToUint32());
      return true;
    }
    return false;
  }

  template <class K, class V>
  bool operator>>(std::pair<K, V>& pair) const {
    const Object pairObject;
    if (operator>>(const_cast<Object&>(pairObject))) {
      pairObject[MapKey] >> pair.first;
      pairObject[MapValue] >> pair.second;
      return true;
    }
    return false;
  }

  template <class T>
  bool operator>>(std::vector<T>& container) const {
    return GetContainerSingle(container);
  }

  template <class T>
  bool operator>>(std::deque<T>& container) const {
    return GetContainerSingle(container);
  }

  template <class T>
  bool operator>>(std::list<T>& container) const {
    return GetContainerSingle(container);
  }

  template <class T, class P, class A>
  bool operator>>(std::set<T, P, A>& container) const {
    return GetContainerSingleSet(container);
  }

  template <class T>
  bool operator>>(std::unordered_set<T>& container) const {
    return GetContainerSingleSet(container);
  }

  template <class K, class V>
  bool operator>>(std::map<K, V>& container) const {
    return GetContainerPaired(container);
  }

  template <class K, class V>
  bool operator>>(std::unordered_map<K, V>& container) const {
    return GetContainerPaired(container);
  }

 protected:
  template <class T>
  bool GetContainerSingle(T& container) const {
    Array array;
    if (operator>>(array)) {
      for (const auto& value : array) {
        typename T::value_type value_;
        OutputValue(value) >> (value_);
        container.emplace(container.end(), value_);
      }
      return true;
    }
    return false;
  }

  template <class T>
  bool GetContainerSingleSet(T& container) const {
    Array array;
    if (operator>>(array)) {
      for (const auto& value : array) {
        typename T::value_type value_;
        OutputValue(value) >> (value_);
        container.emplace(value_);
      }
      return true;
    }
    return false;
  }

  template <class T>
  bool GetContainerPaired(T& container) const {
    Array array;
    if (operator>>(array)) {
      for (const auto& value : array) {
        const Object pairObject;
        OutputValue(value) >> const_cast<Object&>(pairObject);
        typename T::key_type keyValue_;
        typename T::mapped_type mappedValue_;
        pairObject[MapKey] >> keyValue_;
        pairObject[MapValue] >> mappedValue_;
        container.emplace(keyValue_, mappedValue_);
      }
      return true;
    }
    return false;
  }

 private:
  const Value& m_value;
};

}  // namespace JSON
}  // namespace Gengine
