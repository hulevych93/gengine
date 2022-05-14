#pragma once

#include <interprocess-communication/CommunicationEngine.h>
#include <interprocess-communication/InterprocessCommonDefs.h>

namespace Gengine {
namespace InterprocessCommunication {

/**
 * @brief The InterprocessAcceptor class
 *
 * Interface for interprocess connections acception.
 */
class InterprocessAcceptor {
 public:
  /**
   * Connected callback.
   * @param success it the client is connected.
   * @param channel of communication.
   */
  using connected_callback =
      std::function<void(bool success, std::unique_ptr<IChannel>&& channel)>;

 public:
  virtual ~InterprocessAcceptor() = default;

  /**
   * @brief AcceptConnection
   *
   * Asynchronous acception. Call is not blocking.
   *
   * @param connected_callback
   */
  virtual void AcceptConnection(connected_callback callback) = 0;
};

/**
 * @brief makeAcceptor
 * @param connectionString is the local pipe address or unix domain socket file.
 * @param engine for async request processing.
 * @return InterprocessAcceptor instance.
 */
std::unique_ptr<InterprocessAcceptor> makeAcceptor(
    const std::wstring& connectionString,
    const std::shared_ptr<CommunicationEngine>& engine);

}  // namespace InterprocessCommunication
}  // namespace Gengine
