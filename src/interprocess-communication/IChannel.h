#pragma once

#include <interprocess-communication/InterprocessCommonDefs.h>
#include <string>

namespace Gengine {
namespace InterprocessCommunication {

/**
 * IChannel is the interface for interprocess data transport.
 * It implements two ways of send/receive operations: synchornus and
 * asyncronous.
 */
class IChannel {
 public:
  /**
   * Virtual destructor.
   */
  virtual ~IChannel() = default;

  /**
   * Disconnect operation closes a handle/unix file and disconnects a channel.
   */
  virtual void Disconnect() = 0;

  /**
   * IsConnected operation tests if the channel is connected.
   * @return true the channel is connected.
   */
  virtual bool IsConnected() const = 0;

  /**
   * Synchronous send operation on the channel.
   * The execution flow will block until the data has been sent.
   * @param[in] data is a pointer to a data buffer to be sent.
   * @param[in] size is a size of the buffer to be sent.
   * @param[out] returns a number of bytes sent, may be null in case it is
   * unneeded.
   * @return true on successfull send operation.
   */
  virtual bool Send(const void* data,
                    std::uint32_t size,
                    std::uint32_t* bytesProccessed = nullptr) = 0;

  /**
   * Synchronous recv operation on the channel.
   * The execution flow will block until the data has been received.
   * @param[in] data is a pointer to a data buffer to be received.
   * @param[in] size is a size of the buffer to be received.
   * @param[out] returns a number of bytes processed, may be null in case it is
   * unneeded.
   * @return true on successfull receive operation.
   */
  virtual bool Recv(void* data,
                    std::uint32_t size,
                    std::uint32_t* bytesProccessed = nullptr) = 0;

  /**
   * Asyncronus send operation on the channel. No execution block occurs.
   * @param[in] data is a pointer to a data buffer to be sent.
   * @param[in] size is a size of the buffer to be sent.
   * @return true if operation is successfully posted on execution.
   */
  virtual bool SendAsync(const void* data, std::uint32_t size) = 0;

  /**
   * Asyncronus receive operation on the channel. No execution block occurs.
   * @param[in] data is a pointer to a data buffer to be received.
   * @param[in] size is a size of the buffer to be received.
   * @return true if operation is successfully posted on execution.
   */
  virtual bool RecvAsync(void* data, std::uint32_t size) = 0;
};

/**
 * IChannel implementation factory function.
 * @param[in] connectionString could be either windows named pipe name or
 * unix socket file name string.
 * @return ready to use (e.g. connected) channel instance.
 */
std::unique_ptr<IChannel> makeChannel(const std::wstring& connectionString);

}  // namespace InterprocessCommunication
}  // namespace Gengine
