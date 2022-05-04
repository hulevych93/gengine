#include <core/Encoding.h>
#include <json/DetailValue.h>
#include <json/Parser.h>
#include <json/Value.h>

#include <ostream>
#include <sstream>

namespace Gengine {
namespace JSON {

using namespace details;

Value::Value() : m_value(NullValue{}) {}

Value::~Value() = default;

Value::Value(const Value& that) : m_value(that.m_value) {}

Value::Value(Value&& that) : m_value(std::move(that.m_value)) {}

Value::Value(const Object& value) : m_value(ObjectValue{value}) {}

Value::Value(const Array& value) : m_value(ArrayValue{value}) {}

Value::Value(const Number& value) : m_value(NumberValue{value}) {}

Value::Value(const char_t* value) : m_value(StringValue{value}) {}

Value::Value(const string_t& value) : m_value(StringValue{value}) {}

Value::Value(real_t value) : m_value(NumberValue{value}) {}

Value::Value(int64_t value) : m_value(NumberValue{value}) {}

Value::Value(uint64_t value) : m_value(NumberValue{value}) {}

Value::Value(int32_t value)
    : m_value(NumberValue(static_cast<int64_t>(value))) {}

Value::Value(uint32_t value)
    : m_value(NumberValue(static_cast<uint64_t>(value))) {}

Value::Value(int8_t value)
    : m_value(NumberValue(static_cast<int64_t>(value))) {}

Value::Value(uint8_t value)
    : m_value(NumberValue(static_cast<uint64_t>(value))) {}

Value::Value(int16_t value)
    : m_value(NumberValue(static_cast<int64_t>(value))) {}

Value::Value(uint16_t value)
    : m_value(NumberValue(static_cast<uint64_t>(value))) {}

Value::Value(bool value) : m_value(BoolValue{value}) {}

type_t Value::Type() const {
  return boost::apply_visitor([](auto&& value) { return value.Type(); },
                              m_value);
}

bool Value::operator==(const Value& that) const {
  return m_value == that.m_value;
}

Value& Value::operator=(const Value& that) {
  if (this != &that) {
    m_value = that.m_value;
  }
  return *this;
}

Value& Value::operator=(Value&& that) {
  if (this != &that) {
    m_value.swap(that.m_value);
  }
  return *this;
}

bool Value::IsNull() const {
  return Type() == type_t::TypeNull;
}

bool Value::IsString() const {
  return Type() == type_t::TypeString;
}

bool Value::IsNumber() const {
  return Type() == type_t::TypeNumber;
}

bool Value::IsObject() const {
  return Type() == type_t::TypeObject;
}

bool Value::IsArray() const {
  return Type() == type_t::TypeArray;
}

bool Value::IsBool() const {
  return Type() == type_t::TypeBool;
}

void Value::Serialize(stream_t& stream) const {
  boost::apply_visitor(
      [&stream](auto&& value) { return value.Serialize(stream); }, m_value);
}

void Value::Serialize(string_t& stream) const {
  std::stringstream stream_;
  Serialize(stream_);
  stream = stream_.str();
}

void Value::Deserialize(const string_t& stream) {
  Parser parser;
  parser.Parse(stream, *this);
}

const string_t& Value::ToString() const {
  return boost::get<StringValue>(m_value).ToString();
}

const Number& Value::ToNumber() const {
  return boost::get<NumberValue>(m_value).ToNumber();
}

const Object& Value::ToObject() const {
  return boost::get<ObjectValue>(m_value).ToObject();
}

const Array& Value::ToArray() const {
  return boost::get<ArrayValue>(m_value).ToArray();
}

bool Value::ToBool() const {
  return boost::get<BoolValue>(m_value).ToBool();
}

uint64_t Value::ToUint64() const {
  return ToNumber().ToUint64();
}

int64_t Value::ToInt64() const {
  return ToNumber().ToInt64();
}

real_t Value::ToReal() const {
  return ToNumber().ToReal();
}

}  // namespace JSON
}  // namespace Gengine
