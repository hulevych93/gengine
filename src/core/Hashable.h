#pragma once

#include <functional>

#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Gengine {

inline void hash_combine(std::size_t& seed) {}

inline void hash_combine(std::size_t& seed, boost::blank) {}

template <typename T, typename... Rest>
inline void hash_combine(std::size_t& seed, const T& v, Rest... rest) {
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  Gengine::hash_combine(seed, rest...);
}

}  // namespace Gengine

namespace std {
template <class T, class V>
struct hash<std::pair<T, V>> {
  std::size_t operator()(const std::pair<T, V>& t) const {
    std::size_t ret = 0;
    Gengine::hash_combine(ret, t.first, t.second);
    return ret;
  }
};

template <class T, class V>
struct hash<std::map<T, V>> {
  std::size_t operator()(const std::map<T, V>& t) const {
    std::size_t ret = 0;
    for (const auto& iter : t)
      Gengine::hash_combine(ret, iter.first, iter.second);
    return ret;
  }
};

template <class T, class V>
struct hash<std::unordered_map<T, V>> {
  std::size_t operator()(const std::unordered_map<T, V>& t) const {
    std::size_t ret = 0;
    for (const auto& iter : t)
      Gengine::hash_combine(ret, iter.first, iter.second);
    return ret;
  }
};

template <class T>
struct hash<std::vector<T>> {
  std::size_t operator()(const std::vector<T>& t) const {
    std::size_t ret = 0;
    for (const auto& iter : t)
      Gengine::hash_combine(ret, iter);
    return ret;
  }
};

template <class T>
struct hash<std::set<T>> {
  std::size_t operator()(const std::set<T>& t) const {
    std::size_t ret = 0;
    for (const auto& iter : t)
      Gengine::hash_combine(ret, iter);
    return ret;
  }
};

template <class T>
struct hash<std::unordered_set<T>> {
  std::size_t operator()(const std::unordered_set<T>& t) const {
    std::size_t ret = 0;
    for (const auto& iter : t)
      Gengine::hash_combine(ret, iter);
    return ret;
  }
};

template <class T>
struct hash<boost::optional<T>> {
  std::size_t operator()(const boost::optional<T>& t) const {
    std::size_t ret = 0;
    if (t)
      Gengine::hash_combine(ret, t.get());
    return ret;
  }
};

static const struct VariantHasher : boost::static_visitor<std::size_t> {
  template <class T>
  std::size_t operator()(const T& value) const {
    std::size_t ret = 0;
    Gengine::hash_combine(ret, value);
    return ret;
  }
} variantHasher;

template <class... T>
struct hash<boost::variant<T...>> {
  std::size_t operator()(const boost::variant<T...>& t) const {
    return boost::apply_visitor(variantHasher, t);
  }
};
}  // namespace std

#define MAKE_HASHABLE(type, ...)                  \
  namespace std {                                 \
  template <>                                     \
  struct hash<type> {                             \
    std::size_t operator()(const type& t) const { \
      std::size_t ret = 0;                        \
      Gengine::hash_combine(ret, __VA_ARGS__);    \
      return ret;                                 \
    }                                             \
  };                                              \
  }
