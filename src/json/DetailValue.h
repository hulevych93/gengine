#pragma once

#include <json/Common.h>
#include <json/Number.h>

#include <boost/variant.hpp>

#include <unordered_map>

namespace Gengine {
namespace JSON {
namespace details {

class NullValue final {
 public:
  NullValue() = default;

  type_t Type() const;
  void Serialize(stream_t& stream) const;

  bool operator==(const NullValue& that) const;
};

class StringValue final {
 public:
  StringValue(const string_t& value);

  const string_t& ToString() const;

  type_t Type() const;
  void Serialize(stream_t& stream) const;

  bool operator==(const StringValue& that) const;

 private:
  string_t m_value;

 private:
  static string_t HandleUnescapedChars(const string_t& value);
  static const std::unordered_map<string_t, string_t> EspaceChars;
};

class NumberValue final {
 public:
  NumberValue(const Number& value);

  const Number& ToNumber() const;

  type_t Type() const;
  void Serialize(stream_t& stream) const;

  bool operator==(const NumberValue& that) const;

 private:
  Number m_value;
};

class ObjectValue final {
 public:
  ObjectValue(const Object& that);
  ObjectValue(const ObjectValue& value);
  ObjectValue(ObjectValue&& value);
  ~ObjectValue();

  ObjectValue& operator=(const ObjectValue& that);
  ObjectValue& operator=(ObjectValue&& that);

  const Object& ToObject() const;

  type_t Type() const;
  void Serialize(stream_t& stream) const;

  bool operator==(const ObjectValue& that) const;

 private:
  std::unique_ptr<Object> m_value;
};

class ArrayValue final {
 public:
  ArrayValue(const Array& value);
  ArrayValue(const ArrayValue& value);
  ArrayValue(ArrayValue&& value);
  ~ArrayValue();

  ArrayValue& operator=(const ArrayValue& that);
  ArrayValue& operator=(ArrayValue&& that);

  const Array& ToArray() const;

  type_t Type() const;
  void Serialize(stream_t& stream) const;

  bool operator==(const ArrayValue& that) const;

 private:
  std::unique_ptr<Array> m_value;
};

class BoolValue final {
 public:
  BoolValue(bool value);

  bool ToBool() const;

  type_t Type() const;
  void Serialize(stream_t& stream) const;

  bool operator==(const BoolValue& that) const;

 private:
  bool m_value;
};

using value_t = boost::variant<NullValue,
                               StringValue,
                               NumberValue,
                               BoolValue,
                               ArrayValue,
                               ObjectValue>;

}  // namespace details
}  // namespace JSON
}  // namespace Gengine
