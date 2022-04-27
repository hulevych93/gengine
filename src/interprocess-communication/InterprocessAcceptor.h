#pragma once

#include <interprocess-communication/InterprocessCommonDefs.h>

namespace Gengine {
namespace InterprocessCommunication {
class InterprocessServer;

class InterprocessAcceptor
{
public:
    virtual ~InterprocessAcceptor() = default;
    virtual void AcceptConnection(connected_callback callback) = 0;
};
}
}