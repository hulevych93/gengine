#pragma once

#include <serialization/Common.h>
#include <core/VariantCreator.h>

namespace Gengine {
namespace Serialization {

class ISerializable;

class Deserializer: public boost::static_visitor<bool>
{
public:
    using TSize = std::uint32_t;
    using TGetter = std::function<void(const std::uint8_t* data, size_t size)>;

public:
    Deserializer(const Blob& blob);
    Deserializer(const std::shared_ptr<Blob>& blob);
    Deserializer(const void* data, size_t size);

    bool operator>>(std::size_t& data) const;
    bool operator>>(std::uint8_t&  data) const;
    bool operator>>(std::uint16_t& data) const;
    bool operator>>(std::uint32_t& data) const;
    bool operator>>(std::uint64_t& data) const;
    bool operator>>(std::int8_t&  data) const;
    bool operator>>(std::int16_t& data) const;
    bool operator>>(std::int32_t& data) const;
    bool operator>>(std::int64_t& data) const;
    bool operator>>(bool& data) const;
    bool operator>>(Blob& blob) const;
    bool operator>>(ISerializable& serializable) const;

    template<class T>
    typename std::enable_if<std::is_enum<T>::value, bool>::type operator>>(T& data) const
    {
        return GetFixed(&data, sizeof(data));
    }

    template<class _Elem, class _Traits, class _Alloc>
    bool operator>>(std::basic_string<_Elem, _Traits, _Alloc>& string) const
    {
        return GetSized([&](const std::uint8_t* data, size_t size) {
            string.assign(reinterpret_cast<const _Elem*>(data), size / sizeof(_Elem));
        });
    }

    template<class T>
    bool operator>>(std::unique_ptr<T>& object) const
    {
        std::int8_t res = { 0 };
        bool result = operator>>(res);
        if (res > 0)
        {
            object = std::make_unique<T>();
            result &= operator>>(*object);
        }
        return result;
    }

    template<class T>
    bool operator>>(std::shared_ptr<T>& value) const
    {
        std::int8_t res = { 0 };
        bool result = operator>>(res);
        if (res > 0)
        {
            result &= GetShared(value, std::integral_constant<bool, std::is_final<T>::value || std::is_fundamental<T>::value>{});
        }
        return result;
    }

    template<class T>
    bool operator>>(boost::optional<T>& object) const
    {
        std::int8_t res = { 0 };
        bool result = operator>>(res);
        if (res > 0)
        {
            T value;
            result &= operator>>(value);
            object = value;
        }
        return result;
    }

    template<class... T>
    bool operator>>(boost::variant<T...>& object) const
    {
        std::int32_t which{ 0 };
        if (operator>>(which))
        {
            create_variant(which, object);
            return boost::apply_visitor(*this, object);
        }
        return false;
    }

    template<class T>
    bool operator()(T& operand) const
    {
        return operator>>(operand);
    }

    template<class T>
    bool operator>>(std::vector<T>& container) const
    {
        return GetContainerSingle(container);
    }

    template<class T>
    bool operator>>(std::deque<T>& container) const
    {
        return GetContainerSingle(container);
    }

    template<class T>
    bool operator>>(std::list<T>& container) const
    {
        return GetContainerSingle(container);
    }

    template<class T>
    bool operator>>(std::set<T>& container) const
    {
        return GetContainerSingleSet(container);
    }

    template<class T>
    bool operator>>(std::unordered_set<T>& container) const
    {
        return GetContainerSingleSet(container);
    }

    template<class K, class V>
    bool operator>>(std::map<K, V>& container) const
    {
        return GetContainerPaired(container);
    }

    template<class T, class P, class A>
    bool operator>>(std::set<T, P, A>& container) const
    {
        return GetContainerSingleSet(container);
    }

    template<class K, class V>
    bool operator>>(std::unordered_map<K, V>& container) const
    {
        return GetContainerPaired(container);
    }

    template<class K, class V>
    bool operator>>(std::pair<K, V>& pair) const
    {
        bool result = operator>>(pair.first);
        result &= operator>>(pair.second);
        return result;
    }

protected:
    bool PeekFixed(void* peek, size_t size) const;
    bool GetFixed(void* get, size_t size) const;
    bool GetSized(TGetter getter) const;
    bool GetSize(size_t& size) const;

    size_t Used() const;

    template<class T>
    bool GetContainerSingle(T& container) const
    {
        size_t size;
        bool result = GetSize(size);

        for (size_t i = 0; i < size; i++)
        {
            typename T::value_type value;
            operator>>(value);
            container.emplace(container.end(), value);
        }
        return result;
    }

    template<class T>
    bool GetContainerSingleSet(T& container) const
    {
        size_t size;
        bool result = GetSize(size);

        for (size_t i = 0; i < size; i++)
        {
            typename T::value_type value;
            operator>>(value);
            container.emplace(value);
        }
        return result;
    }

    template<class T>
    bool GetContainerPaired(T& container) const
    {
        size_t size;
        bool result = GetSize(size);

        for (size_t i = 0; i < size; i++)
        {
            typename T::key_type key;
            typename T::mapped_type mapped;
            operator>>(key);
            operator>>(mapped);
            container.emplace(key, mapped);
        }
        return result;
    }

    template<class T>
    bool GetShared(std::shared_ptr<T>& value, std::true_type) const
    {
        value = std::make_shared<T>();
        return operator>>(*value);
    }

    template<class T>
    bool GetShared(std::shared_ptr<T>& value, std::false_type) const
    {
        std::uint32_t type = 0;
        PeekFixed(&type, sizeof(type));
        value = T::Create(type);
        return operator>>(*value);
    }

private:
    const std::uint8_t* const m_base;
    mutable std::uint8_t* m_data;
    const size_t m_size;
};
}
}
