#pragma once

#include <cstdint>
#include <memory>
#include <sstream>
#include <string>

namespace Gengine {
namespace JSON {

enum class type_t : std::uint8_t {
  TypeNull = 0,
  TypeString,
  TypeNumber,
  TypeObject,
  TypeArray,
  TypeBool
};

using char_t = char;
using wstring_t = std::basic_string<wchar_t>;
using string_t = std::basic_string<char_t>;
using stream_t = std::basic_ostream<char_t>;
using key_t = string_t;

using int64_t = std::int64_t;
using uint64_t = std::uint64_t;
using int32_t = std::int32_t;
using uint32_t = std::uint32_t;
using int16_t = std::int16_t;
using uint16_t = std::uint16_t;
using int8_t = std::int8_t;
using uint8_t = std::uint8_t;
using real_t = double;

class Value;
class Number;
class Object;
class Array;
class InputValue;
class OutputValue;

namespace details {
class NullValue;
class ObjectValue;
class StringValue;
class NumberValue;
class ArrayValue;
}  // namespace details

using detail_value_t = std::unique_ptr<details::NullValue>;

class IJsonSerializable {
 public:
  virtual bool Serialize(Object& serializer) const = 0;
  virtual bool Deserialize(const Object& deserializer) = 0;
};

static const char* MapKey = "key";
static const char* MapValue = "value";

static const char* VariantKey = "which";
static const char* VariantData = "data";
}  // namespace JSON
}  // namespace Gengine
