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

namespace Gengine {
namespace InterprocessCommunication {


#if 1
//handle of local socket on Linux os
typedef int RPC_FILE_HANDLE;
#define PIPE_PREFIX L"/tmp/com.gengine."
#elif defined(BUILD_WINDOWS)
//HANDLE of named pipe
typedef void* RPC_FILE_HANDLE;
#define PIPE_PREFIX L"\\\\.\\Pipe\\"
#else 
#error "ERROR! Unknown build config!"
#endif

#define INVALID_RPC_FILE (RPC_FILE_HANDLE)(~0)

class IChannel;
class CommunicationEngine;

using interface_key = std::string;

using ipc_connection = boost::variant<PipeConnection, TcpConnection>;
using connected_callback = std::function<void(bool, std::unique_ptr<IChannel>&&)>;

using accepted_callback = std::function<void()>;
using readwrite_callback = std::function<void(bool, std::uint32_t, bool)>;

using engine_callback = boost::variant<accepted_callback, readwrite_callback>;

enum class ParametersTypes : std::uint8_t
{
    TYPE_INT8 = 0,
    TYPE_INT16,
    TYPE_INT32,
    TYPE_INT64,
    TYPE_UINT8,
    TYPE_UINT16,
    TYPE_UINT32,
    TYPE_UINT64,
    TYPE_WSTRING,
    TYPE_STRING,
    TYPE_BLOB,
    TYPE_BOOL,
    TYPE_PTR,
    TYPE_SINGLE_CONTAINEER,
    TYPE_PAIRED_CONTAINEER,
    BINARY_SERIALIZABLE,
    JSON_SERIALIZABLE
};

enum class ResponseCodes : std::uint8_t
{
    CODE_OK = 0,
    CODE_REQUEST_ERROR,
    CODE_INVALID_REQUEST,
    CODE_UNKNOWN_FUNCTION,
    CODE_PARAMETERS_MISMATCH,
    CODE_UNKNOWN_INTERFACE
};

struct RequestHeader final
{
    std::uint32_t requestDataSize = 0;
    std::uint8_t functionCode = 0;
    char interfaceKey[9];
    bool request = false;
};

struct ResponseHeader
{
    std::uint32_t responseDataSize;
    ResponseCodes responseCode;
};

struct ParameterHeader
{
    std::uint32_t parameterSize;
    ParametersTypes parameterType;
};

}
}
