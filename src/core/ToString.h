#pragma once

#include <core/Encoding.h>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Gengine {

template <class T>
std::string ToString(const T& value);
template <class... T>
std::string ToString(const boost::variant<T...>& value);
template <class T>
std::string ToString(const std::shared_ptr<T>& value);
template <class T>
std::string ToString(const std::unique_ptr<T>& value);
template <class T>
std::string ToString(const boost::optional<T>& value);
static std::string ToString(boost::blank);

template <int I>
struct ToStringChoice : ToStringChoice<I + 1> {};
template <>
struct ToStringChoice<100> {};

template <class T>
auto ToStringHelper(const T& value, ToStringChoice<0>)
    -> decltype(value.ToString()) {
  return value.ToString();
}

template <class T>
auto ToStringHelper(const T& value, ToStringChoice<1>)
    -> decltype(std::string(value)) {
  return std::string(value);
}

template <class T>
auto ToStringHelper(const T& value, ToStringChoice<2>)
    -> decltype(std::to_string(value)) {
  return std::to_string(value);
}

template <class T>
auto ToStringHelper(const T& value, ToStringChoice<3>)
    -> decltype(Gengine::toUtf8(value)) {
  return Gengine::toUtf8(value);
}

template <class T>
std::string ToStringHelper(
    T value,
    ToStringChoice<99>,
    std::enable_if_t<std::is_enum<T>::value, void*> = 0) {
  return ToString(static_cast<std::underlying_type_t<T>>(value));
}

template <class T>
std::string ToStringSingleHelper(const T& value) {
  std::string os;
  os += '[';
  for (const auto& item : value) {
    os += ToString(item);
    os += ',';
  }
  os.pop_back();
  os += ']';
  return os;
}

template <class T>
std::string ToStringPairedHelper(const T& value) {
  std::string os;
  os += '[';
  for (const auto& item : value) {
    os += '{';
    os += ToString(item.first);
    os += ',';
    os += ToString(item.second);
    os += "},";
  }
  os.pop_back();
  os += ']';
  return os;
}

template <class T, class A>
std::string ToStringHelper(const std::vector<T, A>& value, ToStringChoice<5>) {
  return ToStringSingleHelper(value);
}

template <class T, class A>
std::string ToStringHelper(const std::list<T, A>& value, ToStringChoice<6>) {
  return ToStringSingleHelper(value);
}

template <class T, class A>
std::string ToStringHelper(const std::deque<T, A>& value, ToStringChoice<7>) {
  return ToStringSingleHelper(value);
}

template <class T, class A>
std::string ToStringHelper(const std::set<T, A>& value, ToStringChoice<8>) {
  return ToStringSingleHelper(value);
}

template <class T, class A>
std::string ToStringHelper(const std::unordered_set<T, A>& value,
                           ToStringChoice<9>) {
  return ToStringSingleHelper(value);
}

template <class T, class V, class A>
std::string ToStringHelper(const std::map<T, V, A>& value, ToStringChoice<10>) {
  return ToStringPairedHelper(value);
}

template <class T, class V, class A>
std::string ToStringHelper(const std::unordered_map<T, V, A>& value,
                           ToStringChoice<11>) {
  return ToStringPairedHelper(value);
}

template <class T>
std::string ToString(const T& value) {
  return ToStringHelper(value, ToStringChoice<0>{});
}

template <class T>
std::string ToString(const std::shared_ptr<T>& value) {
  if (value)
    return ToString(*value);
  else
    return std::string();
}

template <class T>
std::string ToString(const std::unique_ptr<T>& value) {
  if (value)
    return ToString(*value);
  else
    return std::string();
}

template <class T>
std::string ToString(const boost::optional<T>& value) {
  if (value)
    return ToString(value.get());
  else
    return std::string();
}

static std::string ToString(boost::blank) {
  return "null";
}

static const struct VariantToString : boost::static_visitor<std::string> {
  template <class T>
  std::string operator()(const T& value) const {
    return ToString(value);
  }
} variantToString;

template <class... T>
std::string ToString(const boost::variant<T...>& value) {
  return boost::apply_visitor(variantToString, value);
}

}  // namespace Gengine

namespace std {
template <class T, class V>
bool operator<(const std::unordered_set<T, V>& left,
               const std::unordered_set<T, V>& right) {
  return false;
}

template <class T, class V>
bool operator<(const std::unordered_map<T, V>& left,
               const std::unordered_map<T, V>& right) {
  return false;
}

}  // namespace std
