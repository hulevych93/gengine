#pragma once

#include <cstdint>

#include <interprocess-communication/ResponseCodes.h>

namespace Gengine {
namespace InterprocessCommunication {

/**
 * @brief The RequestHeader struct
 *
 * The request header is send first to the server to give a protocol info of
 * how to read and interpret the information received from the client.
 */
struct RequestHeader final {
  /**
   * @brief requestDataSize tells how many bytes server should read after the
   * header as payload data.
   */
  std::uint32_t requestDataSize = 0;

  /**
   * @brief functionCode tells which one function from interface the server
   * should execute.
   */
  std::uint8_t functionCode = 0;

  /**
   * @brief interfaceKey tells which interface should be executed. (see
   * interface_key)
   */
  char interfaceKey[9];

  /**
   * @brief request tells if the server will send responce data or just event
   * will be processed.
   */
  bool request = false;
};

/**
 * @brief The ResponseHeader struct
 *
 * The server sends responce to a client after each request.
 */
struct ResponseHeader final {
  /**
   * @brief responseDataSize is the payload data size.
   */
  std::uint32_t responseDataSize = 0u;

  /**
   * @brief responseCode if the request processed okey.
   */
  ResponseCodes responseCode = ResponseCodes::Ok;
};

}  // namespace InterprocessCommunication
}  // namespace Gengine
