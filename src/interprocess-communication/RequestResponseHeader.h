#pragma once

#include <cstdint>

#include <interprocess-communication/ResponseCodes.h>

namespace Gengine {
namespace InterprocessCommunication {

struct RequestHeader final {
  std::uint32_t requestDataSize = 0;
  std::uint8_t functionCode = 0;
  char interfaceKey[9];
  bool request = false;
};

struct ResponseHeader final {
  std::uint32_t responseDataSize = 0u;
  ResponseCodes responseCode = ResponseCodes::Ok;
};

}  // namespace InterprocessCommunication
}  // namespace Gengine
