#pragma once

#include <interprocess-communication/tcp/sockets/Socket.h>

namespace Gengine {
namespace Network {
class TCPSocket : public Socket<boost::asio::ip::tcp::socket> {
 public:
  TCPSocket(boost::asio::io_context& ioService);
  ~TCPSocket();

  void SetTimeout(std::size_t seconds) override;
  void AsyncConnect(const std::string& address,
                    std::uint16_t port,
                    connect_cb cb) override;
  bool Connect(const std::string& address, std::uint16_t port) override;
  bool IsConnected() const override;
  void Close() override;

 private:
  boost::asio::ip::tcp::resolver m_resolver;
};
}  // namespace Network
}  // namespace Gengine
