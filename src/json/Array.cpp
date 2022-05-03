#include <json/Array.h>
#include <json/Value.h>

#include <iterator>

namespace Gengine {
namespace JSON {

Array::Array() = default;

Array::Array(const storage_type& values) {
  std::copy(values.begin(), values.end(), std::back_inserter(m_values));
}

Array::Array(size_type size) : m_values(size) {}

Array::iterator Array::begin() {
  return m_values.begin();
}

Array::iterator Array::end() {
  return m_values.end();
}

Array::const_iterator Array::begin() const {
  return m_values.begin();
}

Array::const_iterator Array::end() const {
  return m_values.end();
}

Array::reverse_iterator Array::rbegin() {
  return m_values.rbegin();
}

Array::reverse_iterator Array::rend() {
  return m_values.rend();
}

Array::const_reverse_iterator Array::rbegin() const {
  return m_values.rbegin();
}

Array::const_reverse_iterator Array::rend() const {
  return m_values.rend();
}

Array::iterator Array::erase(iterator position) {
  return m_values.erase(position);
}

void Array::erase(size_type index) {
  if (index >= m_values.size()) {
    throw std::out_of_range("index out of bounds");
  }
  m_values.erase(m_values.begin() + index);
}

Value& Array::at(size_type index) {
  return m_values.at(index);
}

const Value& Array::at(size_type index) const {
  return m_values.at(index);
}

Value& Array::operator[](size_type index) {
  return m_values[index];
}

void Array::push_back(const Value& value) {
  m_values.push_back(value);
}

bool Array::empty() const {
  return m_values.empty();
}

void Array::Serialize(stream_t& stream) const {
  stream << '[';
  auto values = m_values;
  if (!values.empty()) {
    auto lastElement = values.end() - 1;
    for (auto iter = values.begin(); iter != lastElement; ++iter) {
      iter->Serialize(stream);
      stream << ',';
    }
    lastElement->Serialize(stream);
  }
  stream << ']';
}

const Value& Array::operator[](size_type index) const {
  return m_values[index];
}

bool Array::operator==(const Array& that) const {
  return m_values == that.m_values;
}

Array::size_type Array::size() const {
  return m_values.size();
}

}  // namespace JSON
}  // namespace Gengine
