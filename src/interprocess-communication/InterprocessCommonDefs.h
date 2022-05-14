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

/**
 * Using request client and server can map the interface client and
 * implementation on the server side.
 */
using interface_key = std::string;

/**
 * The gengine provides two types of ipc. One is for local communication on the
 * same host and the secong one is over network TCP/IP.
 */
using ipc_connection = boost::variant<PipeConnection, TcpConnection>;

}  // namespace InterprocessCommunication
}  // namespace Gengine
