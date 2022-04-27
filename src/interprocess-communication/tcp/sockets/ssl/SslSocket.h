#pragma once

#include <boost/asio/ssl.hpp>
#include "Socket.h"

namespace Core {
namespace Network {
class SslSocket : public Socket<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>
{
public:
    using start_session_cb_t = std::function<void(const boost::system::error_code&)>;

public:
    SslSocket(boost::asio::io_service& ioService);
    ~SslSocket();

    bool IsSuccess(const boost::system::error_code&) const override;
    void SetTimeout(std::size_t seconds) override;
    void AsyncConnect(const std::string& address, std::uint16_t port, connect_cb cb) override;
    bool Connect(const std::string& address, std::uint16_t port) override;
    bool IsConnected() const override;
    void Close() override;

private:
    static boost::asio::ssl::context ClientContext;
};
}
}