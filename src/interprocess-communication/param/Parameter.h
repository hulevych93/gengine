#pragma once

#include <interprocess-communication/InterprocessCommonDefs.h>

namespace Gengine {
namespace InterprocessCommunication {

enum class ParametersTypes : std::uint8_t {
  Int8 = 0,
  Int16,
  Int32,
  Int64,
  UInt8,
  UInt16,
  UInt32,
  UInt64,
  WideString,
  String,
  Blob,
  Boolean,
  RawPtr,
  Container,
  Map,
  BinarySerializable,
  JsonSerializable
};

struct ParameterHeader final {
  std::uint32_t parameterSize = 0u;
  ParametersTypes parameterType = ParametersTypes::Int8;
};

}  // namespace InterprocessCommunication
}  // namespace Gengine
