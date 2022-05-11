#pragma once

#include <api/connection/PipeConnection.h>
#include <api/connection/TcpConnection.h>
#include <core/ToString.h>
#include <json/JSON.h>
#include <boost/variant.hpp>
#include <cstdint>
#include <deque>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <interprocess-communication/HandleType.h>

namespace Gengine {
namespace InterprocessCommunication {

class IChannel;
class CommunicationEngine;

using interface_key = std::string;
using ipc_connection = boost::variant<PipeConnection, TcpConnection>;
using connected_callback =
    std::function<void(bool success, std::unique_ptr<IChannel>&& channel)>;
using accepted_callback = std::function<void()>;
using readwrite_callback =
    std::function<void(bool success, std::uint32_t bytesProcessed, bool)>;
using engine_callback = boost::variant<accepted_callback, readwrite_callback>;

enum class ResponseCodes : std::uint8_t {
  Ok = 0,
  RequestError,
  InvalidRequest,
  UnknownFunction,
  ParametersMismatch,
  UnknownInterface
};

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
