#pragma once

#include <vector>
#include <json/Common.h>

namespace Gengine {
namespace JSON {

class Object
{
public:
    using value_type = std::pair<key_t, Value>;
    using input_value = InputValue;
    using output_value = OutputValue;
    using storage_type = std::vector<value_type>;
    using iterator = storage_type::iterator;
    using const_iterator = storage_type::const_iterator;
    using reverse_iterator = storage_type::reverse_iterator;
    using const_reverse_iterator = storage_type::const_reverse_iterator;
    using size_type = storage_type::size_type;

public:
    Object();
    Object(Object&& that);
    Object(const Object& that);
    Object(const storage_type& values);
    ~Object();

    Object& operator=(const Object& that);
    Object& operator=(Object&& that);

public:
    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;
    reverse_iterator rbegin();
    const_reverse_iterator rbegin() const;
    reverse_iterator rend();
    const_reverse_iterator rend() const;
    std::pair<iterator, bool> insert(const value_type& value);
    iterator erase(iterator position);
    void erase(const key_t& key);
    input_value at(const key_t& key);
    output_value at(const key_t& key) const;
    bool operator==(const Object& that) const;
    input_value operator[](const key_t& key);
    output_value operator[](const key_t& key) const;
    size_type size() const;

    void Serialize(string_t& stream) const;
    void Serialize(stream_t& stream) const;

private:
    iterator find_by_key(const key_t& key);
    const_iterator find_by_key(const key_t& key) const;
    void SerializeKey(stream_t& stream, const key_t& key) const;

private:
    storage_type m_values;

    friend class Value;
};

}
}