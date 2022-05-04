#include "DetailValue.h"

#include <json/Array.h>
#include <json/Object.h>

#include <boost/algorithm/string/replace.hpp>

namespace Gengine {
namespace JSON {
namespace details {

type_t NullValue::Type() const {
  return type_t::TypeNull;
}

void NullValue::Serialize(stream_t& stream) const {
  stream << "null";
}

bool NullValue::operator==(const NullValue&) const {
  return false;
}

// StringValue
StringValue::StringValue(const string_t& value) : m_value(value) {}

type_t StringValue::Type() const {
  return type_t::TypeString;
}

void StringValue::Serialize(stream_t& stream) const {
  stream << '\"' << HandleUnescapedChars(m_value) << '\"';
}

const string_t& StringValue::ToString() const {
  return m_value;
}

bool StringValue::operator==(const StringValue& that) const {
  return m_value == that.m_value;
}

string_t details::StringValue::HandleUnescapedChars(const string_t& value) {
  auto handledValue(value);
  for (const auto& escapeSymbolIter : EspaceChars) {
    boost::replace_all(handledValue, escapeSymbolIter.first,
                       escapeSymbolIter.second);
  }
  return handledValue;
}

const std::unordered_map<string_t, string_t> details::StringValue::EspaceChars =
    {{"\b", "\\b"}, {"\\", "\\\\"}, {"\f", "\\f"}, {"\r", "\\r"},
     {"\n", "\\n"}, {"\t", "\\t"},  {"\"", "\\\""}};

NumberValue::NumberValue(const Number& value) : m_value(value) {}

type_t NumberValue::Type() const {
  return type_t::TypeNumber;
}

void NumberValue::Serialize(stream_t& stream) const {
  m_value.Serialize(stream);
}

const Number& NumberValue::ToNumber() const {
  return m_value;
}

bool NumberValue::operator==(const NumberValue& that) const {
  return m_value == that.m_value;
}

ObjectValue::ObjectValue(const Object& that)
    : m_value(std::make_unique<Object>(that)) {}

ObjectValue::ObjectValue(const ObjectValue& value) {
  *this = value;
}

ObjectValue::ObjectValue(ObjectValue&& value) = default;

ObjectValue::~ObjectValue() = default;

type_t ObjectValue::Type() const {
  return type_t::TypeObject;
}

void ObjectValue::Serialize(stream_t& stream) const {
  m_value->Serialize(stream);
}

bool ObjectValue::operator==(const ObjectValue& that) const {
  if (m_value != that.m_value) {
    if (m_value && that.m_value) {
      return *m_value == *that.m_value;
    }
    return false;
  }
  return true;
}

const Object& ObjectValue::ToObject() const {
  return *m_value;
}

ObjectValue& ObjectValue::operator=(const ObjectValue& that) {
  if (this != &that) {
    if (that.m_value) {
      m_value = std::make_unique<Object>(*that.m_value);
    } else {
      m_value.reset();
    }
  }
  return *this;
}

ObjectValue& ObjectValue::operator=(ObjectValue&& that) = default;

ArrayValue::ArrayValue(const Array& value)
    : m_value(std::make_unique<Array>(value)) {}

ArrayValue::ArrayValue(const ArrayValue& value) {
  *this = value;
}

ArrayValue::ArrayValue(ArrayValue&& value) = default;

ArrayValue::~ArrayValue() {}

type_t ArrayValue::Type() const {
  return type_t::TypeArray;
}

ArrayValue& ArrayValue::operator=(const ArrayValue& that) {
  if (this != &that) {
    if (that.m_value) {
      m_value = std::make_unique<Array>(*that.m_value);
    } else {
      m_value.reset();
    }
  }
  return *this;
}

ArrayValue& ArrayValue::operator=(ArrayValue&& that) = default;

bool ArrayValue::operator==(const ArrayValue& that) const {
  if (m_value != that.m_value) {
    if (m_value && that.m_value) {
      return *m_value == *that.m_value;
    }
    return false;
  }
  return true;
}

void ArrayValue::Serialize(stream_t& stream) const {
  m_value->Serialize(stream);
}

const Array& ArrayValue::ToArray() const {
  return *m_value;
}

BoolValue::BoolValue(const bool value) : m_value(value) {}

type_t BoolValue::Type() const {
  return type_t::TypeBool;
}

void BoolValue::Serialize(stream_t& stream) const {
  stream << (m_value ? "true" : "false");
}

bool BoolValue::operator==(const BoolValue& that) const {
  return m_value == that.m_value;
}

bool BoolValue::ToBool() const {
  return m_value;
}

}  // namespace details
}  // namespace JSON
}  // namespace Gengine
