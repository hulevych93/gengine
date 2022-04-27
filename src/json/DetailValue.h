#pragma once

#include <unordered_map>
#include <json/Number.h>
#include <json/Object.h>
#include <json/Array.h>

namespace Gengine {
namespace JSON {
namespace details {

class NullValue
{
public:
    NullValue();
    virtual ~NullValue();

    virtual type_t Type() const;
    virtual void Serialize(stream_t& stream) const;

    virtual const string_t& ToString() const;
    virtual const Number& ToNumber() const;
    virtual const Object& ToObject() const;
    virtual const Array& ToArray() const;
    virtual bool ToBool() const;
    virtual bool IsEquals(const detail_value_t& that) const;
    virtual detail_value_t Copy() const;

private:
    static string_t NullLiteral();
};

class StringValue : public NullValue
{
public:
    StringValue(const string_t& value);
    virtual ~StringValue();

    type_t Type() const override;
    void Serialize(stream_t& stream) const override;
    const string_t& ToString() const override;
    bool IsEquals(const detail_value_t& that) const override;
    detail_value_t Copy() const override;

private:
    string_t m_value;

private:
    static string_t HandleUnescapedChars(const string_t& value);
    static const std::unordered_map<string_t, string_t> EspaceChars;
};

class NumberValue : public NullValue
{
public:
    NumberValue(const Number& value);
    virtual ~NumberValue();

    type_t Type() const override;
    void Serialize(stream_t& stream) const override;
    const Number& ToNumber() const override;
    bool IsEquals(const detail_value_t& that) const override;
    detail_value_t Copy() const override;

private:
    Number m_value;
};

class ObjectValue : public NullValue
{
public:
    ObjectValue(const Object& that);
    virtual ~ObjectValue();

    type_t Type() const override;
    void Serialize(stream_t& stream) const override;
    const Object& ToObject() const override;
    bool IsEquals(const detail_value_t& that) const override;
    detail_value_t Copy() const override;

private:
    Object m_value;
};

class ArrayValue : public NullValue
{
public:
    ArrayValue(const Array& value);
    virtual ~ArrayValue();

    type_t Type() const override;
    void Serialize(stream_t& stream) const override;
    const Array& ToArray() const override;
    bool IsEquals(const detail_value_t& that) const override;
    detail_value_t Copy() const override;

private:
    Array m_value;
};

class BoolValue : public NullValue
{
public:
    BoolValue(const bool& value);
    virtual ~BoolValue();

    type_t Type() const override;
    void Serialize(stream_t& stream) const override;
    bool ToBool() const override;
    bool IsEquals(const detail_value_t& that) const override;
    detail_value_t Copy() const override;

private:
    static string_t TrueLiteral();
    static string_t FalseLiteral();

private:
    bool m_value;
};
} // details

}
}

