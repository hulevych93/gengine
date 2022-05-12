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
#include <interprocess-communication/RequestResponseHeader.h>
#include <interprocess-communication/ResponseCodes.h>

namespace Gengine {
namespace InterprocessCommunication {

using interface_key = std::string;
using ipc_connection = boost::variant<PipeConnection, TcpConnection>;

using accepted_callback = std::function<void()>;
using readwrite_callback =
    std::function<void(bool success, std::uint32_t bytesProcessed, bool)>;
using engine_callback = boost::variant<accepted_callback, readwrite_callback>;

}  // namespace InterprocessCommunication
}  // namespace Gengine
