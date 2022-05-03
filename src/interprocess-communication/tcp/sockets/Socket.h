#pragma once

#include <interprocess-communication/tcp/sockets/ISocket.h>

namespace Gengine {
namespace Network {
template <class SocketType>
class Socket : public ISocket,
               public std::enable_shared_from_this<Socket<SocketType>> {
 public:
  template <class... Args>
  Socket(Args&&... args)
      : m_socket(std::forward<Args>(args)...),
        m_strand(std::forward<Args>(args)...) {}

  bool IsSuccess(const boost::system::error_code& err) const override {
    return err == boost::asio::error::eof;
  }

  void AsyncWrite(const boost::asio::const_buffer& buff,
                  ISocket::write_callback_t cb) override {
    auto self =
        std::enable_shared_from_this<Socket<SocketType>>::shared_from_this();
    m_strand.post([buff, cb, this, self]() {
      boost::asio::async_write(
          m_socket, buff,
          m_strand.wrap([self, cb](const boost::system::error_code& ec,
                                   size_t bytesSent) { cb(ec, bytesSent); }));
    });
  }

  void AsyncRead(boost::asio::streambuf& buff,
                 read_callback_t cb,
                 size_t bytes = 1) override {
    auto self =
        std::enable_shared_from_this<Socket<SocketType>>::shared_from_this();
    boost::asio::async_read(
        m_socket, buff, boost::asio::transfer_exactly(bytes),
        m_strand.wrap([self, cb](const boost::system::error_code& ec,
                                 size_t bytesSent) { cb(ec, bytesSent); }));
  }

  void AsyncReadUntil(boost::asio::streambuf& buff,
                      const std::string& marker,
                      read_callback_t cb) override {
    auto self =
        std::enable_shared_from_this<Socket<SocketType>>::shared_from_this();
    boost::asio::async_read_until(
        m_socket, buff, marker,
        m_strand.wrap([self, cb](const boost::system::error_code& ec,
                                 size_t bytesSent) { cb(ec, bytesSent); }));
  }

  void AsyncReadAtLeast(boost::asio::streambuf& buff,
                        read_callback_t cb,
                        size_t bytes = 1) override {
    auto self =
        std::enable_shared_from_this<Socket<SocketType>>::shared_from_this();
    boost::asio::async_read(
        m_socket, buff, boost::asio::transfer_at_least(bytes),
        m_strand.wrap([self, cb](const boost::system::error_code& ec,
                                 size_t bytesSent) { cb(ec, bytesSent); }));
  }

  void AsyncReadAll(boost::asio::streambuf& buff, read_callback_t cb) override {
    auto self =
        std::enable_shared_from_this<Socket<SocketType>>::shared_from_this();
    boost::asio::async_read(
        m_socket, buff, boost::asio::transfer_all(),
        m_strand.wrap([self, cb](const boost::system::error_code& ec,
                                 size_t bytesSent) { cb(ec, bytesSent); }));
  }

  std::size_t Write(const boost::asio::const_buffer& buff) override {
    return boost::asio::write(m_socket, buff);
  }

  std::size_t Read(boost::asio::streambuf& buff, size_t bytes = 1) override {
    return boost::asio::read(m_socket, buff,
                             boost::asio::transfer_exactly(bytes));
  }

  std::size_t ReadAtLeast(boost::asio::streambuf& buff,
                          size_t bytes = 1) override {
    return boost::asio::read(m_socket, buff,
                             boost::asio::transfer_at_least(bytes));
  }

  std::size_t ReadUntil(boost::asio::streambuf& buff,
                        const std::string& marker) override {
    return boost::asio::read_until(m_socket, buff, marker);
  }

  std::size_t ReadAll(boost::asio::streambuf& buff) override {
    return boost::asio::read(m_socket, buff, boost::asio::transfer_all());
  }

 public:
  SocketType m_socket;
  boost::asio::io_context::strand m_strand;
};
}  // namespace Network
}  // namespace Gengine
