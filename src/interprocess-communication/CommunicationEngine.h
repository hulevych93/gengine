#pragma once

#include <interprocess-communication/InterprocessCommonDefs.h>

#include <core/Runnable.h>

namespace Gengine {
namespace InterprocessCommunication {
class IChannel;

/**
 * @brief The CommunicationEngine class
 *
 * Class for asyncronous communication (e.g. boost::asio::io_context.)
 */
class CommunicationEngine : public Runnable {
 public:
  /**
   * The event is acception of the new client.
   */
  using accepted_callback = std::function<void()>;

  /**
   * The event is read/write
   * @param if the error occur
   * @param how many bytes processed
   */
  using readwrite_callback =
      std::function<void(bool success, std::uint32_t bytesProcessed, bool)>;

  /**
   * async engine callback
   */
  using engine_callback = boost::variant<accepted_callback, readwrite_callback>;

 public:
  virtual ~CommunicationEngine() = default;

  /**
   * @brief RegisterConnection
   *
   * Register channel for async communication.
   * @param connection is the channel.
   * @param callback of the async operation.
   */
  virtual void RegisterConnection(const IChannel& connection,
                                  engine_callback callback) = 0;

  /**
   * @brief UnregisterConnection
   *
   * Unregisted the channel.
   * @param connection is the channel.
   */
  virtual void UnregisterConnection(const IChannel& connection) = 0;
};

/**
 * @brief makeEngine
 * @param[in] threadId of the worker thread in which the async requests will be
 * processed.
 * @return The CommunicationEngine instance.
 */
std::unique_ptr<CommunicationEngine> makeEngine(std::uint32_t threadId);

}  // namespace InterprocessCommunication
}  // namespace Gengine
