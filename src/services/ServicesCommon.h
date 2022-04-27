#pragma once

#include <interprocess-communication/InterprocessCommonDefs.h>
#include <interprocess-syncronization/InterprocessSynchronizationCommon.h>
#include <api/connection/ExternalConnection.h>
#include <api/connection/SharedConnection.h>

namespace Gengine {
namespace Services {

using service_connection = boost::variant<boost::blank, ExternalConnection, InterprocessCommunication::ipc_connection, SharedConnection>;
using service_location = boost::variant<boost::blank, InterprocessSynchronization::executable_params>;

}
}
