#pragma once

#include <boost/asio.hpp>
#include <functional>

namespace Gengine {
namespace Network {
class ISocket {
 public:
  using connect_cb = std::function<void(const boost::system::error_code&)>;
  using write_callback_t =
      std::function<void(const boost::system::error_code&, size_t bytesSent)>;
  using read_callback_t =
      std::function<void(const boost::system::error_code& ec,
                         size_t bytesReceived)>;

 public:
  virtual ~ISocket() {}

  virtual bool IsSuccess(const boost::system::error_code&) const = 0;
  virtual void SetTimeout(std::size_t seconds) = 0;
  virtual void AsyncConnect(const std::string& address,
                            std::uint16_t port,
                            connect_cb cb) = 0;
  virtual bool Connect(const std::string& address, std::uint16_t port) = 0;
  virtual bool IsConnected() const = 0;
  virtual void Close() = 0;

  virtual void AsyncWrite(const boost::asio::const_buffer& buff,
                          write_callback_t cb) = 0;
  virtual void AsyncRead(boost::asio::streambuf& buff,
                         read_callback_t cb,
                         size_t bytes = 1) = 0;
  virtual void AsyncReadUntil(boost::asio::streambuf& buff,
                              const std::string& marker,
                              read_callback_t cb) = 0;
  virtual void AsyncReadAll(boost::asio::streambuf& buff,
                            read_callback_t cb) = 0;
  virtual void AsyncReadAtLeast(boost::asio::streambuf& buff,
                                read_callback_t cb,
                                size_t bytes = 1) = 0;

  virtual std::size_t Write(const boost::asio::const_buffer& buff) = 0;
  virtual std::size_t Read(boost::asio::streambuf& buff, size_t bytes = 1) = 0;
  virtual std::size_t ReadAtLeast(boost::asio::streambuf& buff,
                                  size_t bytes = 1) = 0;
  virtual std::size_t ReadUntil(boost::asio::streambuf& buff,
                                const std::string& marker) = 0;
  virtual std::size_t ReadAll(boost::asio::streambuf& buff) = 0;
};
}  // namespace Network
}  // namespace Gengine