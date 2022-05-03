#include <json/Number.h>

namespace Gengine {
namespace JSON {

Number::Number(real_t value)
    : m_type(Number::type_t::real_type), m_realValue(value) {}

Number::Number(int8_t value)
    : m_type(Number::type_t::signed_type),
      m_intValue(static_cast<int64_t>(value)) {}

Number::Number(uint8_t value)
    : m_type(Number::type_t::unsigned_type),
      m_uintValue(static_cast<uint64_t>(value)) {}

Number::Number(int16_t value)
    : m_type(Number::type_t::signed_type),
      m_intValue(static_cast<int64_t>(value)) {}

Number::Number(uint16_t value)
    : m_type(Number::type_t::unsigned_type),
      m_uintValue(static_cast<uint64_t>(value)) {}

Number::Number(int32_t value)
    : m_type(Number::type_t::signed_type),
      m_intValue(static_cast<int64_t>(value)) {}

Number::Number(uint32_t value)
    : m_type(Number::type_t::unsigned_type),
      m_uintValue(static_cast<uint64_t>(value)) {}

Number::Number(int64_t value)
    : m_type(Number::type_t::signed_type), m_intValue(value) {}

Number::Number(uint64_t value)
    : m_type(Number::type_t::unsigned_type), m_uintValue(value) {}

Number::~Number() {}

bool Number::IsReal() const {
  return m_type == Number::type_t::real_type;
}

bool Number::IsUint64() const {
  bool result = 0;
  switch (m_type) {
    case Number::type_t::signed_type:
      result = m_intValue >= 0;
      break;
    case Number::type_t::unsigned_type:
      result = true;
      break;
    default:
      result = false;
      break;
  }
  return result;
}

real_t Number::ToReal() const {
  real_t result = 0.0;
  switch (m_type) {
    case Number::type_t::real_type:
      result = m_realValue;
      break;
    case Number::type_t::signed_type:
      result = static_cast<real_t>(m_intValue);
      break;
    case Number::type_t::unsigned_type:
      result = static_cast<real_t>(m_uintValue);
      break;
    default:
      throw std::logic_error("Unknown number type");
  }
  return result;
}

uint64_t Number::ToUint64() const {
  uint64_t result = 0;
  switch (m_type) {
    case Number::type_t::real_type:
      result = static_cast<uint64_t>(m_realValue);
      break;
    case Number::type_t::signed_type:
      result = static_cast<uint64_t>(m_intValue);
      break;
    case Number::type_t::unsigned_type:
      result = m_uintValue;
      break;
    default:
      throw std::logic_error("Unknown number type");
  }
  return result;
}

int64_t Number::ToInt64() const {
  int64_t result = 0;
  switch (m_type) {
    case Number::type_t::real_type:
      result = static_cast<int64_t>(m_realValue);
      break;
    case Number::type_t::signed_type:
      result = m_intValue;
      break;
    case Number::type_t::unsigned_type:
      result = static_cast<int64_t>(m_uintValue);
      break;
    default:
      throw std::logic_error("Unknown number type");
  }
  return result;
}

uint32_t Number::ToUint32() const {
  uint32_t result = 0;
  switch (m_type) {
    case Number::type_t::real_type:
      result = static_cast<uint32_t>(m_realValue);
      break;
    case Number::type_t::signed_type:
      result = static_cast<uint32_t>(m_intValue);
      break;
    case Number::type_t::unsigned_type:
      result = static_cast<uint32_t>(m_uintValue);
      break;
    default:
      throw std::logic_error("Unknown number type");
  }
  return result;
}

int32_t Number::ToInt32() const {
  int32_t result = 0;
  switch (m_type) {
    case Number::type_t::real_type:
      result = static_cast<uint32_t>(m_realValue);
      break;
    case Number::type_t::signed_type:
      result = static_cast<uint32_t>(m_intValue);
      break;
    case Number::type_t::unsigned_type:
      result = static_cast<uint32_t>(m_uintValue);
      break;
    default:
      throw std::logic_error("Unknown number type");
  }
  return result;
}

bool Number::operator==(const Number& that) const {
  return m_intValue == that.m_intValue || m_realValue == that.m_realValue ||
         m_uintValue == that.m_uintValue;
}

void Number::Serialize(stream_t& stream) const {
  switch (m_type) {
    case Number::type_t::real_type:
      stream << std::to_string(m_realValue);
      break;
    case Number::type_t::signed_type:
      stream << std::to_string(m_intValue);
      break;
    case Number::type_t::unsigned_type:
      stream << std::to_string(m_uintValue);
      break;
    default:
      throw std::logic_error("Unknown number type");
  }
}

}  // namespace JSON
}  // namespace Gengine
