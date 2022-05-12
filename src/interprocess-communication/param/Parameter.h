#pragma once

#include <interprocess-communication/InterprocessCommonDefs.h>

namespace Gengine {
namespace InterprocessCommunication {

/**
 * @brief The ParametersTypes enum
 *
 * The parameter types that are used during interprocess communication.
 * See InputParameters, OutputParameters.
 */
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

/**
 * @brief The ParameterHeader struct
 *
 * The parameter header consists of actual binary parameter size and it's type.
 * The size is checked during communication over network.
 */
struct ParameterHeader final {
  /**
   * @brief parameterSize the size of parameter on the target machine.
   */
  std::uint32_t parameterSize = 0u;

  /**
   * @brief parameterType of parameter in the target machine.
   */
  ParametersTypes parameterType = ParametersTypes::Int8;
};

}  // namespace InterprocessCommunication
}  // namespace Gengine
