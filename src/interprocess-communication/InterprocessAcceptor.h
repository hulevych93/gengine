#pragma once

#include <interprocess-communication/CommunicationEngine.h>
#include <interprocess-communication/InterprocessCommonDefs.h>

namespace Gengine {
namespace InterprocessCommunication {
class InterprocessServer;

using connected_callback =
    std::function<void(bool success, std::unique_ptr<IChannel>&& channel)>;

class InterprocessAcceptor {
 public:
  virtual ~InterprocessAcceptor() = default;
  virtual void AcceptConnection(connected_callback callback) = 0;
};

std::unique_ptr<InterprocessAcceptor> makeAcceptor(
    const std::wstring& connectionString,
    const std::shared_ptr<CommunicationEngine>& engine);

}  // namespace InterprocessCommunication
}  // namespace Gengine
