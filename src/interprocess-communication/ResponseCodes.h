#pragma once

#include <cstdint>

namespace Gengine {
namespace InterprocessCommunication {

/**
 * @brief The ResponseCodes enum
 */
enum class ResponseCodes : std::uint8_t {
  Ok = 0,
  RequestError,
  InvalidRequest,
  UnknownFunction,
  ParametersMismatch,
  UnknownInterface
};

}  // namespace InterprocessCommunication
}  // namespace Gengine
