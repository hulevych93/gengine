#pragma once

#include <core/Blob.h>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

template <class T>
struct is_trivially_serializable
    : std::integral_constant<bool,
                             std::is_object<T>::value &&
                                 std::is_standard_layout<T>::value &&
                                 std::alignment_of<T>::value == 1> {};

#define DECLARE_NO_POLYMORPHIC(TYPE)                                         \
  static std::shared_ptr<TYPE> Create() { return std::make_shared<TYPE>(); } \
  static std::shared_ptr<TYPE> Create(                                       \
      Serialization::Deserializer& deserializer) {                           \
    try {                                                                    \
      auto instance = std::make_shared<TYPE>();                              \
      instance->Deserialize(deserializer);                                   \
      return instance;                                                       \
    } catch (...) {                                                          \
    }                                                                        \
    return std::shared_ptr<TYPE>();                                          \
  }
