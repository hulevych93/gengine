#pragma once

#include <vector>
#include <json/Common.h>
#include <json/Value.h>

namespace Gengine {
namespace JSON {

class Array
{
    typedef std::vector<Value> storage_type;

public:
    typedef storage_type::iterator iterator;
    typedef storage_type::const_iterator const_iterator;
    typedef storage_type::reverse_iterator reverse_iterator;
    typedef storage_type::const_reverse_iterator const_reverse_iterator;
    typedef storage_type::size_type size_type;

public:
    Array();
    Array(size_type size);
    Array(const storage_type& values);

public:
    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;
    reverse_iterator rbegin();
    const_reverse_iterator rbegin() const;
    reverse_iterator rend();
    const_reverse_iterator rend() const;
    iterator erase(iterator position);
    void erase(size_type index);
    Value& at(size_type index);
    const Value& at(size_type index) const;
    Value& operator[](size_type index);
    const Value& operator[](size_type index) const;
    bool operator==(const Array& that) const;
    size_type size() const;
    void push_back(const Value& value);
    bool empty() const;

    void Serialize(stream_t& stream) const;

private:
    storage_type m_values;

    friend class Value;
};

}
}
