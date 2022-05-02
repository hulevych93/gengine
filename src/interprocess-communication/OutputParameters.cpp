#include "OutputParameters.h"

#include <string.h>
#include <wchar.h>
#include <assert.h>

namespace Gengine {
using namespace JSON;
using namespace Serialization;
namespace InterprocessCommunication {
OutputParameters::OutputParameters()
    : m_allocatedSize(0)
    , m_filedSize(0)
{}

void OutputParameters::AllocateBufferSpace(std::uint32_t size, ParameterHeader** header, std::uint8_t** data)
{
    auto sizeRequired = size + sizeof(ParameterHeader);
    sizeRequired += m_filedSize;
    if (m_allocatedSize < sizeRequired)
    {
        auto temp = std::make_unique<std::uint8_t[]>(sizeRequired);
        memcpy(temp.get(), m_buffer.get(), m_filedSize);
        m_buffer = std::move(temp);
    }
    *header = reinterpret_cast<ParameterHeader*>(&m_buffer[m_filedSize]);
    *data = m_buffer.get() + m_filedSize + sizeof(ParameterHeader);
    m_filedSize += size + sizeof(ParameterHeader);
}

template<class Type>
void OutputParameters::AppendIntegerParameter(Type value, ParametersTypes type)
{
    std::uint8_t* dest = nullptr;
    ParameterHeader* header = nullptr;
    AllocateBufferSpace(sizeof(value), &header, &dest);
    header->parameterSize = sizeof(value);
    header->parameterType = type;
    *reinterpret_cast<Type*>(dest) = value;
}

void OutputParameters::Append(bool value)
{
    AppendIntegerParameter(value, ParametersTypes::Boolean);
}

void OutputParameters::Append(void* value)
{
    std::uint8_t* dest = nullptr;
    ParameterHeader* header = nullptr;
    AllocateBufferSpace(sizeof(value), &header, &dest);
    header->parameterSize = sizeof(value);
    header->parameterType = ParametersTypes::RawPtr;
    memcpy(dest, &value, sizeof(value));
}

void OutputParameters::Append(std::int8_t value)
{
    AppendIntegerParameter(value, ParametersTypes::Int8);
}

void OutputParameters::Append(std::int16_t value)
{
    AppendIntegerParameter(value, ParametersTypes::Int16);
}

void OutputParameters::Append(std::int32_t value)
{
    AppendIntegerParameter(value, ParametersTypes::Int32);
}

void OutputParameters::Append(std::int64_t value)
{
    AppendIntegerParameter(value, ParametersTypes::Int64);
}

void OutputParameters::Append(std::uint8_t value)
{
    AppendIntegerParameter(value, ParametersTypes::UInt8);
}

void OutputParameters::Append(std::uint16_t value)
{
    AppendIntegerParameter(value, ParametersTypes::UInt16);
}

void OutputParameters::Append(std::uint32_t value)
{
    AppendIntegerParameter(value, ParametersTypes::UInt32);
}

void OutputParameters::Append(std::uint64_t value)
{
    AppendIntegerParameter(value, ParametersTypes::UInt64);
}

void OutputParameters::Append(const ISerializable& type)
{
    Serializer serializer;
    type.Serialize(serializer);
    auto blob = serializer.GetBlob();
    AppendSizedParameter(blob->GetData(), blob->GetSize(), ParametersTypes::BinarySerializable);
}

void OutputParameters::Append(const IJsonSerializable& type)
{
    Object root;
    type.Serialize(root);
    std::string stream;
    root.Serialize(stream);
    auto size = static_cast<std::uint32_t>(stream.length() * sizeof(char));
    AppendSizedParameter(stream.c_str(), size, ParametersTypes::JsonSerializable);
}

void* OutputParameters::AppendSizedParameter(std::uint32_t size, ParametersTypes type)
{
    void* data = nullptr;
    ParameterHeader* pHeader = nullptr;
    AllocateBufferSpace(size, &pHeader, reinterpret_cast<std::uint8_t**>(&data));
    pHeader->parameterSize = size;
    pHeader->parameterType = type;
    return data;
}

void OutputParameters::AppendSizedParameter(const void* data, std::uint32_t size, ParametersTypes type)
{
    std::uint8_t* dest = nullptr;
    ParameterHeader* pHeader = nullptr;
    AllocateBufferSpace(size, &pHeader, &dest);
    pHeader->parameterSize = size;
    pHeader->parameterType = type;
    memcpy(dest, data, size);
}

void OutputParameters::Append(const std::wstring& value)
{
    auto size = static_cast<std::uint32_t>(value.length() * sizeof(wchar_t));
    AppendSizedParameter(value.c_str(), size, ParametersTypes::WideString);
}

void OutputParameters::Append(const std::string& value)
{
    auto size = static_cast<std::uint32_t>(value.length() * sizeof(char));
    AppendSizedParameter(value.c_str(), size, ParametersTypes::String);
}

void OutputParameters::Append(const Blob& param)
{
    AppendSizedParameter(param.GetData(), param.GetSize(), ParametersTypes::Blob);
}

}
}
