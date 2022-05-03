#pragma once

#include <deque>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Gengine {

template <class type>
bool compare(const type& in, const type& out) {
  return in == out;
}

template <class type>
bool compare(const std::shared_ptr<type>& in,
             const std::shared_ptr<type>& out) {
  return *in == *out;
}

template <class type>
bool compareSingle(const type& in, const type& out) {
  for (const auto& elm : in) {
    auto iter = std::find_if(out.begin(), out.end(),
                             [elm](const typename type::value_type& item) {
                               return compare(elm, item);
                             });
    if (iter == out.end()) {
      return false;
    }
  }
  return true;
}

template <class type>
bool comparePaired(const type& in, const type& out) {
  for (const auto& elm : in) {
    auto iter = std::find_if(out.begin(), out.end(),
                             [elm](const typename type::value_type& item) {
                               return compare(elm.first, item.first) &&
                                      compare(elm.second, item.second);
                             });
    if (iter == out.end()) {
      return false;
    }
  }
  return true;
}

template <class type>
bool compare(const std::vector<type>& in, const std::vector<type>& out) {
  return compareSingle(in, out);
}

template <class type>
bool compare(const std::list<type>& in, const std::list<type>& out) {
  return compareSingle(in, out);
}

template <class type>
bool compare(const std::deque<type>& in, const std::deque<type>& out) {
  return compareSingle(in, out);
}

template <class type>
bool compare(const std::set<type>& in, const std::set<type>& out) {
  return compareSingle(in, out);
}

template <class type>
bool compare(const std::unordered_set<type>& in,
             const std::unordered_set<type>& out) {
  return compareSingle(in, out);
}

template <class type1, class type2>
bool compare(const std::map<type1, type2>& in,
             const std::map<type1, type2>& out) {
  return comparePaired(in, out);
}

template <class type1, class type2>
bool compare(const std::unordered_map<type1, type2>& in,
             const std::unordered_map<type1, type2>& out) {
  return comparePaired(in, out);
}

}  // namespace Gengine
