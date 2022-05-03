#include <core/Encoding.h>
#include <json/Array.h>
#include <json/Number.h>
#include <json/Object.h>
#include <json/Value.h>
#include <json/ValueAdapter.h>

namespace Gengine {
namespace JSON {

InputValue::InputValue(Value& value) : m_value(value) {}

bool InputValue::operator<<(const IJsonSerializable& value) {
  Object object;
  if (value.Serialize(object)) {
    return operator<<(object);
  }
  return false;
}

bool InputValue::operator<<(const Object& value) {
  m_value = value;
  return true;
}

bool InputValue::operator<<(const Array& value) {
  m_value = value;
  return true;
}

bool InputValue::operator<<(const Number& value) {
  m_value = value;
  return true;
}

bool InputValue::operator<<(const Value& value) {
  m_value = value;
  return true;
}

bool InputValue::operator<<(const char_t* value) {
  m_value = value;
  return true;
}

bool InputValue::operator<<(const string_t& value) {
  m_value = value;
  return true;
}

bool InputValue::operator<<(real_t value) {
  m_value = value;
  return true;
}

bool InputValue::operator<<(uint64_t value) {
  m_value = value;
  return true;
}

bool InputValue::operator<<(int64_t value) {
  m_value = value;
  return true;
}

bool InputValue::operator<<(uint32_t value) {
  m_value = value;
  return true;
}

bool InputValue::operator<<(int32_t value) {
  m_value = value;
  return true;
}

bool InputValue::operator<<(uint16_t value) {
  m_value = value;
  return true;
}

bool InputValue::operator<<(int16_t value) {
  m_value = value;
  return true;
}

bool InputValue::operator<<(uint8_t value) {
  m_value = value;
  return true;
}

bool InputValue::operator<<(int8_t value) {
  m_value = value;
  return true;
}

bool InputValue::operator<<(bool value) {
  m_value = value;
  return true;
}

bool InputValue::operator<<(const wstring_t& value) {
  m_value = toUtf8(value);
  return true;
}

bool InputValue::operator<<(boost::blank value) {
  return true;
}

OutputValue::OutputValue(const Value& value) : m_value(value) {}

bool OutputValue::operator>>(IJsonSerializable& value) const {
  Object object;
  if (operator>>(object)) {
    return value.Deserialize(object);
  }
  return false;
}

bool OutputValue::operator>>(string_t& value) const {
  if (m_value.IsString()) {
    value = m_value.ToString();
    return true;
  }
  return false;
}

bool OutputValue::operator>>(real_t& value) const {
  if (m_value.IsNumber()) {
    value = m_value.ToNumber().ToReal();
    return true;
  }
  return false;
}

bool OutputValue::operator>>(uint64_t& value) const {
  if (m_value.IsNumber()) {
    value = m_value.ToNumber().ToUint64();
    return true;
  }
  return false;
}

bool OutputValue::operator>>(int64_t& value) const {
  if (m_value.IsNumber()) {
    value = m_value.ToNumber().ToInt64();
    return true;
  }
  return false;
}

bool OutputValue::operator>>(uint32_t& value) const {
  if (m_value.IsNumber()) {
    value = m_value.ToNumber().ToUint32();
    return true;
  }
  return false;
}

bool OutputValue::operator>>(int32_t& value) const {
  if (m_value.IsNumber()) {
    value = m_value.ToNumber().ToInt32();
    return true;
  }
  return false;
}

bool OutputValue::operator>>(uint16_t& value) const {
  if (m_value.IsNumber()) {
    value = m_value.ToNumber().ToUint32();
    return true;
  }
  return false;
}

bool OutputValue::operator>>(int16_t& value) const {
  if (m_value.IsNumber()) {
    value = m_value.ToNumber().ToInt32();
    return true;
  }
  return false;
}

bool OutputValue::operator>>(uint8_t& value) const {
  if (m_value.IsNumber()) {
    value = m_value.ToNumber().ToUint32();
    return true;
  }
  return false;
}

bool OutputValue::operator>>(int8_t& value) const {
  if (m_value.IsNumber()) {
    value = m_value.ToNumber().ToInt32();
    return true;
  }
  return false;
}

bool OutputValue::operator>>(Object& value) const {
  if (m_value.IsObject()) {
    value = m_value.ToObject();
    return true;
  }
  return false;
}

bool OutputValue::operator>>(Array& value) const {
  if (m_value.IsArray()) {
    value = m_value.ToArray();
    return true;
  }
  return false;
}

bool OutputValue::operator>>(bool& value) const {
  if (m_value.IsBool()) {
    value = m_value.ToBool();
    return true;
  }
  return false;
}

bool OutputValue::operator>>(wstring_t& value) const {
  string_t value_;
  if (operator>>(value_)) {
    value = utf8toWchar(value_);
    return true;
  }
  return false;
}

bool OutputValue::operator>>(boost::blank& value) const {
  return true;
}

}  // namespace JSON
}  // namespace Gengine
