#include "InputParameters.h"

#include <assert.h>
#include <core/Blob.h>
#include <string.h>

namespace Gengine {
using namespace JSON;
using namespace Serialization;
namespace InterprocessCommunication {
InputParameters::InputParameters() : m_size(0) {}

bool InputParameters::Deserialize(void* data, std::uint32_t size) {
  if (size > 0) {
    m_buffer = std::make_unique<std::uint8_t[]>(size);
    memcpy(m_buffer.get(), data, size);
    m_size = size;

    auto data = m_buffer.get();
    auto left = m_size;
    auto success = true;

    while (left > 0) {
      if (left < sizeof(ParameterHeader)) {
        success = false;
        break;
      }
      auto* header = (ParameterHeader*)data;
      left -= sizeof(ParameterHeader);
      data += sizeof(ParameterHeader);
      // validate parameter type and size
      if (!IsParameterHeaderValid(header)) {
        success = false;
        break;
      }
      if (left < header->parameterSize) {
        success = false;
        break;
      }
      // parameter ok
      m_parameters.push_back(header);
      left -= header->parameterSize;
      data += header->parameterSize;
    }
    if (!success) {
      // error during request parsing;
      // cleanup and return false
      m_parameters.clear();
      m_size = 0;
      return false;
    }
    return true;
  }
  return true;
}

const ParameterHeader* InputParameters::GetParameterHeader(
    std::int8_t index) const {
  if (index < 0 || index >= GetParametersCount()) {
    assert(0);
    return nullptr;
  }
  return m_parameters[index];
}

std::int8_t InputParameters::GetParametersCount() const {
  return static_cast<std::int8_t>(m_parameters.size());
}

bool InputParameters::Get(std::int8_t index, bool& value) const {
  return Get(index, value, ParametersTypes::Boolean);
}

bool InputParameters::Get(std::int8_t index, void*& value) const {
  if (index < 0 || index >= GetParametersCount()) {
    assert(0);
    return false;
  }
  ParameterHeader* header = m_parameters[index];
  if (header->parameterType != ParametersTypes::RawPtr) {
    assert(0);
    return false;
  }

  auto buf = reinterpret_cast<std::uint8_t*>(header);
  buf += sizeof(ParameterHeader);
  value = *reinterpret_cast<void**>(buf);
  return true;
}

bool InputParameters::Get(std::int8_t index, std::uint8_t& value) const {
  return Get(index, value, ParametersTypes::UInt8);
}

bool InputParameters::Get(std::int8_t index, std::uint16_t& value) const {
  return Get(index, value, ParametersTypes::UInt16);
}

bool InputParameters::Get(std::int8_t index, std::uint32_t& value) const {
  return Get(index, value, ParametersTypes::UInt32);
}

bool InputParameters::Get(std::int8_t index, std::uint64_t& value) const {
  return Get(index, value, ParametersTypes::UInt64);
}

bool InputParameters::Get(std::int8_t index, std::int8_t& value) const {
  return Get(index, value, ParametersTypes::Int8);
}

bool InputParameters::Get(std::int8_t index, std::int16_t& value) const {
  return Get(index, value, ParametersTypes::Int16);
}

bool InputParameters::Get(std::int8_t index, std::int32_t& value) const {
  return Get(index, value, ParametersTypes::Int32);
}

bool InputParameters::Get(std::int8_t index, std::int64_t& value) const {
  return Get(index, value, ParametersTypes::Int64);
}

bool InputParameters::Get(std::int8_t index,
                          Serialization::ISerializable& value) const {
  void* data = nullptr;
  std::uint32_t size = 0;
  if (Get(index, &data, &size, ParametersTypes::BinarySerializable)) {
    auto blob = std::make_shared<Blob>(data, size);
    Serialization::Deserializer deserializer(blob);
    value.Deserialize(deserializer);
    return true;
  }

  return false;
}

bool InputParameters::Get(std::int8_t index,
                          JSON::IJsonSerializable& value) const {
  void* data = nullptr;
  std::uint32_t size = 0;
  if (Get(index, &data, &size, ParametersTypes::JsonSerializable)) {
    auto json =
        std::string(reinterpret_cast<const char*>(data), size / sizeof(char));
    Value root;
    root.Deserialize(json);
    value.Deserialize(root.ToObject());
    return true;
  }

  return false;
}

bool InputParameters::Get(std::int8_t index, std::wstring& value) const {
  void* data = nullptr;
  std::uint32_t size = 0;
  if (Get(index, &data, &size, ParametersTypes::WideString)) {
    value = std::wstring(reinterpret_cast<const wchar_t*>(data),
                         size / sizeof(wchar_t));
    return true;
  }

  return false;
}

bool InputParameters::Get(std::int8_t index, std::string& value) const {
  void* data = nullptr;
  std::uint32_t size = 0;
  if (Get(index, &data, &size, ParametersTypes::String)) {
    value =
        std::string(reinterpret_cast<const char*>(data), size / sizeof(char));
    return true;
  }

  return false;
}

bool InputParameters::Get(std::int8_t index, Blob& value) const {
  void* data = nullptr;
  std::uint32_t size = 0;
  if (Get(index, &data, &size, ParametersTypes::Blob)) {
    value = Blob(data, size);
    return true;
  }

  return false;
}

template <class Type>
bool InputParameters::Get(std::int8_t index,
                          Type& value,
                          ParametersTypes type) const {
  if (index < 0 || index >= GetParametersCount()) {
    assert(0);
    return false;
  }
  ParameterHeader* header = m_parameters[index];
  if (header->parameterType != type) {
    assert(0);
    return false;
  }

  auto buf = reinterpret_cast<std::uint8_t*>(header);
  buf += sizeof(ParameterHeader);
  value = *reinterpret_cast<Type*>(buf);
  return true;
}

bool InputParameters::Get(std::int8_t index,
                          void** data,
                          std::uint32_t* size,
                          ParametersTypes type) const {
  if (index < 0 || index >= GetParametersCount()) {
    assert(0);
    return false;
  }
  ParameterHeader* header = m_parameters[index];
  if (header->parameterType != type) {
    assert(0);
    return false;
  }

  auto buffer = reinterpret_cast<::uint8_t*>(header);
  buffer += sizeof(ParameterHeader);
  *size = header->parameterSize;
  *data = buffer;
  return true;
}

bool InputParameters::IsParameterHeaderValid(ParameterHeader* header) {
  switch (header->parameterType) {
    case ParametersTypes::WideString:
      return ((header->parameterSize % sizeof(wchar_t)) == 0);
    case ParametersTypes::Int8:
      return header->parameterSize == sizeof(std::int8_t);
    case ParametersTypes::Int16:
      return header->parameterSize == sizeof(std::int16_t);
    case ParametersTypes::Int32:
      return header->parameterSize == sizeof(std::int32_t);
    case ParametersTypes::Int64:
      return header->parameterSize == sizeof(std::int64_t);
    case ParametersTypes::UInt8:
      return header->parameterSize == sizeof(std::uint8_t);
    case ParametersTypes::UInt16:
      return header->parameterSize == sizeof(std::uint16_t);
    case ParametersTypes::UInt32:
      return header->parameterSize == sizeof(std::uint32_t);
    case ParametersTypes::UInt64:
      return header->parameterSize == sizeof(std::uint64_t);
    case ParametersTypes::Boolean:
      return header->parameterSize == sizeof(bool);
    case ParametersTypes::RawPtr:
      return header->parameterSize == sizeof(void*);
    case ParametersTypes::Blob:
    case ParametersTypes::String:
    case ParametersTypes::Container:
    case ParametersTypes::Map:
    case ParametersTypes::BinarySerializable:
    case ParametersTypes::JsonSerializable:
      return true;
    default:
      // unknown parameter type
      assert(0);
      return false;
  }
}
}  // namespace InterprocessCommunication
}  // namespace Gengine
