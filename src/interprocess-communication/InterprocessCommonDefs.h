#pragma once

#include <memory>
#include <cstdint>
#include <vector>
#include <deque>
#include <list>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <string>
#include <functional>
#include <boost/variant.hpp>
#include <json/JSON.h>
#include <core/ToString.h>
#include <api/connection/PipeConnection.h>
#include <api/connection/TcpConnection.h>

#include <interprocess-communication/HandleType.h>

namespace Gengine {
namespace InterprocessCommunication {

class IChannel;
class CommunicationEngine;

using interface_key = std::string;
using ipc_connection = boost::variant<PipeConnection, TcpConnection>;
using connected_callback = std::function<void(bool success, std::unique_ptr<IChannel>&& channel)>;
using accepted_callback = std::function<void()>;
using readwrite_callback = std::function<void(bool success, std::uint32_t bytesProcessed, bool)>;
using engine_callback = boost::variant<accepted_callback, readwrite_callback>;

enum class ParametersTypes : std::uint8_t
{
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

enum class ResponseCodes : std::uint8_t
{
    Ok = 0,
    RequestError,
    InvalidRequest,
    UnknownFunction,
    ParametersMismatch,
    UnknownInterface
};

struct RequestHeader final
{
    std::uint32_t requestDataSize = 0;
    std::uint8_t functionCode = 0;
    char interfaceKey[9];
    bool request = false;
};

struct ResponseHeader final
{
    std::uint32_t responseDataSize = 0u;
    ResponseCodes responseCode = ResponseCodes::Ok;
};

struct ParameterHeader final
{
    std::uint32_t parameterSize = 0u;
    ParametersTypes parameterType = ParametersTypes::Int8;
};

}
}
