#pragma once

#include <cstdio>
#include <cstdlib>
#include <functional>
#include <type_traits>
#include <string>
#include <vector>
#include <deque>
#include <list>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <memory>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <core/Blob.h>

template<class T>
struct is_trivially_serializable : std::integral_constant < bool, 
    std::is_object<T>::value && std::is_standard_layout<T>::value && std::alignment_of<T>::value == 1> {};

#define DECLARE_NO_POLYMORPHIC(TYPE) \
static std::shared_ptr<TYPE> Create()\
{\
    return std::make_shared<TYPE>();\
}\
static std::shared_ptr<TYPE> Create(Serialization::Deserializer& deserializer)\
{\
    try\
    {\
        auto instance = std::make_shared<TYPE>();\
        instance->Deserialize(deserializer);\
        return instance;\
    }\
    catch(...)\
    {\
    }\
    return std::shared_ptr<TYPE>();\
}

