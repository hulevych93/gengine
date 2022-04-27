#include "InputParameters.h"

#include <core/Blob.h>
#include <assert.h>
#include <string.h>

namespace Gengine {
using namespace JSON;
using namespace Serialization;
namespace InterprocessCommunication {
InputParameters::InputParameters()
    : m_size(0)
{}

bool InputParameters::Deserialize(void* data, std::uint32_t size)
{
    if (size > 0)
    {
        m_buffer = std::make_unique<std::uint8_t[]>(size);
        memcpy(m_buffer.get(), data, size);
        m_size = size;

        auto pCurrentByte = m_buffer.get();
        auto uiSizeLeft = m_size;
        auto bOk = true;//is request parsed successfully?

        while (uiSizeLeft > 0)
        {
            if (uiSizeLeft < sizeof(ParameterHeader))
            {
                assert(0);//invalid data
                bOk = false;
                break;
            }
            ParameterHeader* pHeader = (ParameterHeader*)pCurrentByte;
            uiSizeLeft -= sizeof(ParameterHeader);
            pCurrentByte += sizeof(ParameterHeader);
            if (uiSizeLeft < pHeader->parameterSize)
            {
                assert(0);
                bOk = false;
                break;
            }
            //validate parameter type and size
            if (!IsParameterHeaderValid(pHeader))
            {
                assert(0);
                bOk = false;
                break;
            }
            //parameter ok
            m_parameters.push_back(pHeader);
            uiSizeLeft -= pHeader->parameterSize;
            pCurrentByte += pHeader->parameterSize;
        }
        if (!bOk)
        {
            //error during request parsing;
            //cleanup and return false
            m_parameters.clear();
            m_size = 0;
            return false;
        }
        return true;//parsed successfully
    }
    return true;//empty request is completely legal
}


const ParameterHeader* InputParameters::GetParameterHeader(std::int8_t index) const
{
    if (index < 0 || index >= m_parameters.size())
    {
        assert(0);
        return nullptr;
    }
    return m_parameters[index];
}

std::int8_t InputParameters::GetParametersCount() const
{
    return m_parameters.size();
}

bool InputParameters::Get(std::int8_t index, bool& value) const
{
    return Get(index, value, ParametersTypes::TYPE_BOOL);
}

bool  InputParameters::Get(std::int8_t index, void*& value) const
{
    if (index < 0 || index >= m_parameters.size())
    {
        assert(0);
        return false;
    }
    ParameterHeader* header = m_parameters[index];
    if (header->parameterType != ParametersTypes::TYPE_PTR)
    {
        assert(0);
        return false;
    }

    auto buf = reinterpret_cast<std::uint8_t*>(header);
    buf += sizeof(ParameterHeader);
    value = *reinterpret_cast<void**>(buf);
    return true;
}

bool InputParameters::Get(std::int8_t index, std::uint8_t& value) const
{
    return Get(index, value, ParametersTypes::TYPE_UINT8);
}

bool InputParameters::Get(std::int8_t index, std::uint16_t& value) const
{
    return Get(index, value, ParametersTypes::TYPE_UINT16);
}

bool InputParameters::Get(std::int8_t index, std::uint32_t& value) const
{
    return Get(index, value, ParametersTypes::TYPE_UINT32);
}

bool InputParameters::Get(std::int8_t index, std::uint64_t& value) const
{
    return Get(index, value, ParametersTypes::TYPE_UINT64);
}

bool InputParameters::Get(std::int8_t index, std::int8_t& value) const
{
    return Get(index, value, ParametersTypes::TYPE_INT8);
}

bool InputParameters::Get(std::int8_t index, std::int16_t& value) const
{
    return Get(index, value, ParametersTypes::TYPE_INT16);
}

bool InputParameters::Get(std::int8_t index, std::int32_t& value) const
{
    return Get(index, value, ParametersTypes::TYPE_INT32);
}

bool InputParameters::Get(std::int8_t index, std::int64_t& value) const
{
    return Get(index, value, ParametersTypes::TYPE_INT64);
}

bool InputParameters::Get(std::int8_t index, Serialization::ISerializable& value) const
{
    void* data = nullptr;
    std::uint32_t size = 0;
    if (Get(index, &data, &size, ParametersTypes::BINARY_SERIALIZABLE))
    {
        auto blob = std::make_shared<Blob>(data, size);
        Serialization::Deserializer deserializer(blob);
        value.Deserialize(deserializer);
        return true;
    }

    return false;
}

bool InputParameters::Get(std::int8_t index, JSON::IJsonSerializable& value) const
{
    void* data = nullptr;
    std::uint32_t size = 0;
    if (Get(index, &data, &size, ParametersTypes::JSON_SERIALIZABLE))
    {
        auto json = std::string(reinterpret_cast<const char*>(data), size / sizeof(char));
        Value root;
        root.Deserialize(json);
        value.Deserialize(root.ToObject());
        return true;
    }

    return false;
}

bool InputParameters::Get(std::int8_t index, std::wstring& value) const
{
    void* data = nullptr;
    std::uint32_t size = 0;
    if (Get(index, &data, &size, ParametersTypes::TYPE_WSTRING))
    {
        value = std::wstring(reinterpret_cast<const wchar_t*>(data), size / sizeof(wchar_t));
        return true;
    }

    return false;
}

bool InputParameters::Get(std::int8_t index, std::string& value) const
{
    void* data = nullptr;
    std::uint32_t size = 0;
    if (Get(index, &data, &size, ParametersTypes::TYPE_STRING))
    {
        value = std::string(reinterpret_cast<const char*>(data), size / sizeof(char));
        return true;
    }

    return false;
}

bool InputParameters::Get(std::int8_t index, Blob& value) const
{
    void* data = nullptr;
    std::uint32_t size = 0;
    if (Get(index, &data, &size, ParametersTypes::TYPE_BLOB))
    {
        value = Blob(data, size);
        return true;
    }

    return false;
}

template<class Type>
bool InputParameters::Get(std::int8_t index, Type& value, ParametersTypes type) const
{
    if (index < 0 || index >= m_parameters.size())
    {
        assert(0);
        return false;
    }
    ParameterHeader* header = m_parameters[index];
    if (header->parameterType != type)
    {
        assert(0);
        return false;
    }

    auto buf = reinterpret_cast<std::uint8_t*>(header);
    buf += sizeof(ParameterHeader);
    value = *reinterpret_cast<Type*>(buf);
    return true;
}

bool InputParameters::Get(std::int8_t index, void** data, std::uint32_t* size, ParametersTypes type) const
{
    if (index < 0 || index >= m_parameters.size())
    {
        assert(0);
        return false;
    }
    ParameterHeader* header = m_parameters[index];
    if (header->parameterType != type)
    {
        assert(0);
        return false;
    }

    auto buffer = reinterpret_cast<::uint8_t*>(header);
    buffer += sizeof(ParameterHeader);
    *size = header->parameterSize;
    *data = buffer;
    return true;
}

bool InputParameters::IsParameterHeaderValid(ParameterHeader* header)
{
    switch (header->parameterType)
    {
    case ParametersTypes::TYPE_WSTRING:
        return ((header->parameterSize % sizeof(wchar_t)) == 0);
    case ParametersTypes::TYPE_INT8:
        return header->parameterSize == sizeof(std::int8_t);
    case ParametersTypes::TYPE_INT16:
        return header->parameterSize == sizeof(std::int16_t);
    case ParametersTypes::TYPE_INT32:
        return header->parameterSize == sizeof(std::int32_t);
    case ParametersTypes::TYPE_INT64:
        return header->parameterSize == sizeof(std::int64_t);
    case ParametersTypes::TYPE_UINT8:
        return header->parameterSize == sizeof(std::uint8_t);
    case ParametersTypes::TYPE_UINT16:
        return header->parameterSize == sizeof(std::uint16_t);
    case ParametersTypes::TYPE_UINT32:
        return header->parameterSize == sizeof(std::uint32_t);
    case ParametersTypes::TYPE_UINT64:
        return header->parameterSize == sizeof(std::uint64_t);
    case ParametersTypes::TYPE_BOOL:
        return header->parameterSize == sizeof(bool);
    case ParametersTypes::TYPE_PTR:
        return header->parameterSize == sizeof(void*);
    case ParametersTypes::TYPE_BLOB:
    case ParametersTypes::TYPE_STRING:
    case ParametersTypes::TYPE_SINGLE_CONTAINEER:
    case ParametersTypes::TYPE_PAIRED_CONTAINEER:
    case ParametersTypes::BINARY_SERIALIZABLE:
    case ParametersTypes::JSON_SERIALIZABLE:
        return true;
    default:
        //unknown parameter type
        assert(0);
        return false;
    }
}
}
}
