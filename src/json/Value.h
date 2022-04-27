#pragma once

#include <vector>
#include <json/Common.h>

namespace Gengine {
namespace JSON {

class Value
{
public:
    Value();
    Value(const Value& that);
    Value(Value&& that);
    Value(const char_t* value);
    Value(const string_t& value);
    Value(const Number& value);
    Value(real_t value);
    Value(uint64_t value);
    Value(int64_t value);
    Value(uint32_t value);
    Value(int32_t value);
    Value(uint16_t value);
    Value(int16_t value);
    Value(uint8_t value);
    Value(int8_t value);
    Value(const Object& value);
    Value(const Array& value);
    Value(bool value);
    ~Value();

    bool operator==(const Value& that) const;
    Value &operator=(const Value& that);
    Value &operator=(Value&& that);

    type_t Type() const;

    bool IsNull() const;
    bool IsString() const;
    bool IsNumber() const;
    bool IsObject() const;
    bool IsArray() const;
    bool IsBool() const;

    void Serialize(stream_t& stream) const;
    void Serialize(string_t& stream) const;
    void Deserialize(const string_t& stream);

public:
    const string_t& ToString() const;
    const Number& ToNumber() const;
    const Object& ToObject() const;
    const Array& ToArray() const;
    bool ToBool() const;
    uint64_t ToUint64() const;
    int64_t ToInt64() const;
    real_t ToReal() const;

private:
    detail_value_t m_value;
};

}
}