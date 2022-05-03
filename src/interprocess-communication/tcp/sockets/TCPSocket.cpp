#include "TCPSocket.h"

#include <boost/lexical_cast.hpp>

namespace Gengine {
namespace Network {
TCPSocket::TCPSocket(boost::asio::io_context& ioService)
    : Socket(ioService), m_resolver(ioService) {}

TCPSocket::~TCPSocket() {
  Close();
}

void TCPSocket::SetTimeout(std::size_t seconds) {
#ifdef _WIN32
  setsockopt(m_socket.native_handle(), SOL_SOCKET, SO_RCVTIMEO,
             (const char*)&seconds, sizeof(seconds));
  setsockopt(m_socket.native_handle(), SOL_SOCKET, SO_SNDTIMEO,
             (const char*)&seconds, sizeof(seconds));
#else
/*
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    setsockopt(m_socket.native(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(m_socket.native(), SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    */
#endif
}

void TCPSocket::AsyncConnect(const std::string& address,
                             std::uint16_t port,
                             connect_cb cb) {
  boost::system::error_code error;
  boost::asio::ip::tcp::resolver::query query(
      address, boost::lexical_cast<std::string>(port));
  boost::asio::ip::tcp::resolver::iterator endpoint_iterator =
      m_resolver.resolve(query, error);
  if (!error) {
    boost::asio::async_connect(
        m_socket, endpoint_iterator,
        [cb](const boost::system::error_code& ec,
             boost::asio::ip::tcp::resolver::iterator it) { cb(ec); });
  } else {
    cb(error);
  }
}

bool TCPSocket::Connect(const std::string& address, std::uint16_t port) {
  boost::system::error_code error;
  boost::asio::ip::tcp::resolver::query query(
      address, boost::lexical_cast<std::string>(port));
  boost::asio::ip::tcp::resolver::iterator endpoint_iterator =
      m_resolver.resolve(query, error);
  if (!error) {
    boost::asio::connect(m_socket, endpoint_iterator, error);
    return !error;
  }

  return false;
}

bool TCPSocket::IsConnected() const {
  return m_socket.is_open();
}

void TCPSocket::Close() {
  if (IsConnected()) {
    boost::system::error_code error;
    m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
    m_socket.close(error);
  }
}
}  // namespace Network
}  // namespace Gengine
