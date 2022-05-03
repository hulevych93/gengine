#include "DetailValue.h"

#include <boost/algorithm/string/replace.hpp>

namespace Gengine {
namespace JSON {
namespace details {

// NullValue
NullValue::NullValue() {}

NullValue::~NullValue() {}

type_t NullValue::Type() const {
  return type_t::TypeNull;
}

void NullValue::Serialize(stream_t& stream) const {
  stream << NullLiteral();
}

std::unique_ptr<NullValue> NullValue::Copy() const {
  return std::make_unique<NullValue>();
}

bool NullValue::IsEquals(const detail_value_t& that) const {
  return false;
}

const string_t& NullValue::ToString() const {
  throw std::logic_error("Value is not string");
}

const Number& NullValue::ToNumber() const {
  throw std::logic_error("Value is not number");
}

const Object& NullValue::ToObject() const {
  throw std::logic_error("Value is not object");
}

const Array& NullValue::ToArray() const {
  throw std::logic_error("Value is not array");
}

bool NullValue::ToBool() const {
  throw std::logic_error("Value is not bool");
}

string_t NullValue::NullLiteral() {
  const char_t result[] = {'n', 'u', 'l', 'l'};
  return string_t(result, result + sizeof(result) / sizeof(result[0]));
}

// StringValue
StringValue::StringValue(const string_t& value) : m_value(value) {}

StringValue::~StringValue() {}

type_t StringValue::Type() const {
  return type_t::TypeString;
}

void StringValue::Serialize(stream_t& stream) const {
  stream << '\"' << HandleUnescapedChars(m_value) << '\"';
}

const string_t& StringValue::ToString() const {
  return m_value;
}

std::unique_ptr<NullValue> StringValue::Copy() const {
  return std::make_unique<StringValue>(*this);
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

NumberValue::~NumberValue() {}

type_t NumberValue::Type() const {
  return type_t::TypeNumber;
}

void NumberValue::Serialize(stream_t& stream) const {
  m_value.Serialize(stream);
}

const Number& NumberValue::ToNumber() const {
  return m_value;
}

std::unique_ptr<NullValue> NumberValue::Copy() const {
  return std::make_unique<NumberValue>(*this);
}

ObjectValue::ObjectValue(const Object& that) : m_value(that) {}

ObjectValue::~ObjectValue() {}

type_t ObjectValue::Type() const {
  return type_t::TypeObject;
}

void ObjectValue::Serialize(stream_t& stream) const {
  m_value.Serialize(stream);
}

const Object& ObjectValue::ToObject() const {
  return m_value;
}

std::unique_ptr<NullValue> ObjectValue::Copy() const {
  return std::make_unique<ObjectValue>(*this);
}

ArrayValue::ArrayValue(const Array& value) : m_value(value) {}

ArrayValue::~ArrayValue() {}

type_t ArrayValue::Type() const {
  return type_t::TypeArray;
}

void ArrayValue::Serialize(stream_t& stream) const {
  m_value.Serialize(stream);
}

const Array& ArrayValue::ToArray() const {
  return m_value;
}

std::unique_ptr<NullValue> ArrayValue::Copy() const {
  return std::make_unique<ArrayValue>(*this);
}

BoolValue::BoolValue(const bool& value) : m_value(value) {}

BoolValue::~BoolValue() {}

type_t BoolValue::Type() const {
  return type_t::TypeBool;
}

void BoolValue::Serialize(stream_t& stream) const {
  stream << (m_value ? TrueLiteral() : FalseLiteral());
}

bool BoolValue::ToBool() const {
  return m_value;
}

std::unique_ptr<NullValue> BoolValue::Copy() const {
  return std::make_unique<BoolValue>(*this);
}

string_t BoolValue::TrueLiteral() {
  const char_t result[] = {'t', 'r', 'u', 'e'};
  return string_t(result, result + sizeof(result) / sizeof(result[0]));
}

string_t BoolValue::FalseLiteral() {
  const char_t result[] = {'f', 'a', 'l', 's', 'e'};
  return string_t(result, result + sizeof(result) / sizeof(result[0]));
}

bool details::StringValue::IsEquals(const detail_value_t& that) const {
  if (auto concrete = dynamic_cast<StringValue*>(that.get())) {
    return m_value == concrete->m_value;
  }
  return false;
}

bool details::NumberValue::IsEquals(const detail_value_t& that) const {
  if (auto concrete = dynamic_cast<NumberValue*>(that.get())) {
    return m_value == concrete->m_value;
  }
  return false;
}

bool details::ObjectValue::IsEquals(const detail_value_t& that) const {
  if (auto concrete = dynamic_cast<ObjectValue*>(that.get())) {
    return m_value == concrete->m_value;
  }
  return false;
}

bool details::ArrayValue::IsEquals(const detail_value_t& that) const {
  if (auto concrete = dynamic_cast<ArrayValue*>(that.get())) {
    return m_value == concrete->m_value;
  }
  return false;
}

bool details::BoolValue::IsEquals(const detail_value_t& that) const {
  if (auto concrete = dynamic_cast<BoolValue*>(that.get())) {
    return m_value == concrete->m_value;
  }
  return false;
}

}  // namespace details
}  // namespace JSON
}  // namespace Gengine
