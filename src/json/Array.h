#pragma once

#include <json/Common.h>
#include <json/Value.h>

#include <vector>

namespace Gengine {
namespace JSON {

class Array final {
  using storage_type = std::vector<Value>;

 public:
  using iterator = storage_type::iterator;
  using const_iterator = storage_type::const_iterator;
  using reverse_iterator = storage_type::reverse_iterator;
  using const_reverse_iterator = storage_type::const_reverse_iterator;
  using size_type = storage_type::size_type;

 public:
  Array();
  Array(size_type size);
  Array(const storage_type& values);

 public:
  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;
  reverse_iterator rbegin();
  const_reverse_iterator rbegin() const;
  reverse_iterator rend();
  const_reverse_iterator rend() const;
  iterator erase(iterator position);
  void erase(size_type index);
  Value& at(size_type index);
  const Value& at(size_type index) const;
  Value& operator[](size_type index);
  const Value& operator[](size_type index) const;
  bool operator==(const Array& that) const;
  size_type size() const;
  void push_back(const Value& value);
  bool empty() const;

  void Serialize(stream_t& stream) const;

 private:
  storage_type m_values;

  friend class Value;
};

}  // namespace JSON
}  // namespace Gengine
