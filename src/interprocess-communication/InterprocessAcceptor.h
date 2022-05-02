#pragma once

#include <interprocess-communication/InterprocessCommonDefs.h>
#include <interprocess-communication/CommunicationEngine.h>

namespace Gengine {
namespace InterprocessCommunication {
class InterprocessServer;

class InterprocessAcceptor
{
public:
    virtual ~InterprocessAcceptor() = default;
    virtual void AcceptConnection(connected_callback callback) = 0;
};

std::unique_ptr<InterprocessAcceptor> makeAcceptor(const std::wstring& connectionString,
                                                   const std::shared_ptr<CommunicationEngine>& engine);

}
}
