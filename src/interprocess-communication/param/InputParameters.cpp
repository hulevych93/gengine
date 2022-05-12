#include "InputParameters.h"

#include <assert.h>
#include <core/Blob.h>
#include <string.h>

namespace Gengine {
using namespace JSON;
using namespace Serialization;
namespace InterprocessCommunication {

namespace {

bool IsParameterHeaderValid(ParameterHeader* header) {
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
      return false;
  }
}
}  // namespace

InputParameters::InputParameters() : m_size(0) {}

InputParameters::InputParameters(InputParameters&& that) {
  *this = std::move(that);
}

InputParameters& InputParameters::operator=(InputParameters&& that) {
  if (this != &that) {
    m_parameters = std::move(that.m_parameters);
    m_buffer = std::move(that.m_buffer);
    m_size = that.m_size;
    that.m_size = 0;
  }

  return *this;
}

InputParameters InputParameters::makeParameters(const void* data,
                                                std::uint32_t size) {
  if (size > 0) {
    InputParameters params;
    params.m_size = size;
    params.m_buffer = std::make_unique<std::uint8_t[]>(size);
    memcpy(params.m_buffer.get(), data, size);

    auto data = params.m_buffer.get();
    auto left = size;

    while (left > 0) {
      if (left < sizeof(ParameterHeader)) {
        return InputParameters{};
      }

      auto* header = reinterpret_cast<ParameterHeader*>(data);
      left -= sizeof(ParameterHeader);
      data += sizeof(ParameterHeader);

      if (!IsParameterHeaderValid(header) || left < header->parameterSize) {
        return InputParameters{};
      }

      params.m_parameters.push_back(header);
      left -= header->parameterSize;
      data += header->parameterSize;
    }

    return params;
  }

  return InputParameters{};
}

ParametersTypes InputParameters::GetParameterType(std::int8_t index) const {
  return m_parameters.at(index)->parameterType;
}

std::int8_t InputParameters::GetParametersCount() const {
  return static_cast<std::int8_t>(m_parameters.size());
}

bool InputParameters::Get(std::int8_t index, bool& value) const {
  return Get(index, value, ParametersTypes::Boolean);
}

bool InputParameters::Get(std::int8_t index, void*& value) const {
  if (!CheckInBounds(index)) {
    return false;
  }

  ParameterHeader* header = m_parameters[index];
  if (header->parameterType != ParametersTypes::RawPtr) {
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
    Serialization::Deserializer deserializer(*blob);
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
  if (!CheckInBounds(index)) {
    return false;
  }

  ParameterHeader* header = m_parameters[index];
  if (header->parameterType != type) {
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
  if (!CheckInBounds(index)) {
    return false;
  }

  ParameterHeader* header = m_parameters[index];
  if (header->parameterType != type) {
    return false;
  }

  auto buffer = reinterpret_cast<::uint8_t*>(header);
  buffer += sizeof(ParameterHeader);
  *size = header->parameterSize;
  *data = buffer;
  return true;
}

bool InputParameters::CheckInBounds(std::int8_t index) const noexcept {
  return index >= 0 && index < GetParametersCount();
}

}  // namespace InterprocessCommunication
}  // namespace Gengine
