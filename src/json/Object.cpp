#include <json/Object.h>
#include <json/ValueAdapter.h>
#include <json/Value.h>

#include <algorithm>
#include <iterator>
#include <ostream>
#include <sstream>

namespace Gengine {
namespace JSON {

Object::Object() = default;

Object::Object(const storage_type& values)
{
    m_values.clear();
    std::copy(values.begin(), values.end(), std::back_inserter<storage_type>(m_values));
}

Object::Object(const Object& that)
    : Object(that.m_values)
{

}

Object::Object(Object&& that)
    : m_values(std::move(that.m_values))
{

}

Object::~Object()
{

}

Object& Object::operator=(const Object& that)
{
    if (this != &that)
    {
        m_values.clear();
        std::copy(that.m_values.begin(), that.m_values.end(), std::back_inserter<storage_type>(m_values));
    }
    return *this;
}

Object& Object::operator=(Object&& that)
{
    if (this != &that)
    {
        m_values = std::move(that.m_values);
    }
    return *this;
}

Object::iterator Object::begin()
{
    return m_values.begin();
}

Object::const_iterator Object::begin() const
{
    return m_values.begin();
}

Object::iterator Object::end()
{
    return m_values.end();
}

Object::const_iterator Object::end() const
{
    return m_values.end();
}

Object::reverse_iterator Object::rbegin()
{
    return m_values.rbegin();
}

Object::const_reverse_iterator Object::rbegin() const
{
    return m_values.rbegin();
}

Object::reverse_iterator Object::rend()
{
    return m_values.rend();
}

Object::const_reverse_iterator Object::rend() const
{
    return m_values.rend();
}

std::pair<Object::iterator, bool> Object::insert(const value_type& value)
{
    std::pair<Object::iterator, bool> result;
    result.second = true;
    auto it = find_by_key(value.first);
    if (it != m_values.end())
    {
        it->second = value.second;
        result.second = false;
    }
    {
        it = m_values.insert(it, value);
    }
    result.first = it;
    return result;
}

Object::iterator Object::erase(iterator position)
{
    return m_values.erase(position);
}

void Object::erase(const key_t& key)
{
    auto it = find_by_key(key);
    m_values.erase(it);
}

Object::input_value Object::at(const key_t& key)
{
    auto it = find_by_key(key);
    if (it == m_values.end())
    {
        throw std::out_of_range("Key not found");
    }
    return it->second;
}

Object::output_value Object::at(const key_t& key) const
{
    auto it = find_by_key(key);
    if (it == m_values.end())
    {
        throw std::out_of_range("Key not found");
    }
    return it->second;
}

bool Object::operator==(const Object& that) const
{
    return m_values == that.m_values;
}

Object::input_value Object::operator[](const key_t& key)
{
    auto it = find_by_key(key);
    if (it == m_values.end())
    {
        it = m_values.insert(it, std::pair<key_t, Value>(key, Value()));
    }
    return it->second;
}

Object::output_value Object::operator[](const key_t& key) const
{
    auto it = find_by_key(key);
    if (it != m_values.end())
    {
        return it->second;
    }
    static Value nullValue;
    return nullValue;
}

Object::size_type Object::size() const
{
    return m_values.size();
}

Object::iterator Object::find_by_key(const key_t& key)
{
    return std::find_if(m_values.begin(), m_values.end(),
        [&key](const std::pair<key_t, Value>& p)
    {
        return p.first == key;
    });
}

Object::const_iterator Object::find_by_key(const key_t& key) const
{
    return std::find_if(m_values.begin(), m_values.end(),
        [&key](const std::pair<key_t, Value>& p)
    {
        return p.first == key;
    });
}


void Object::Serialize(stream_t& stream) const
{
    stream << "{";
    auto values = m_values;
    if (!values.empty())
    {
        auto lastElement = values.end() - 1;
        for (auto iter = values.begin(); iter != lastElement; ++iter)
        {
            auto key = iter->first;
            auto value = iter->second;
            SerializeKey(stream, key);
            stream << ':';
            value.Serialize(stream);
            stream << ',';
        }
        SerializeKey(stream, lastElement->first);
        stream << ':';
        lastElement->second.Serialize(stream);
    }
    stream << '}';
}

void Object::Serialize(string_t& stream) const
{
    std::stringstream stream_;
    Serialize(stream_);
    stream = stream_.str();
}

void Object::SerializeKey(stream_t& stream, const key_t& key) const
{
    stream << '\"' << key << '\"';
}

}
}
