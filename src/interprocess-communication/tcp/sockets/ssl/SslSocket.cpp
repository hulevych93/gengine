#include <boost/lexical_cast.hpp>
#include "SslSocket.h"

namespace Core {
namespace Network {
boost::asio::ssl::context SslSocket::ClientContext(boost::asio::ssl::context::sslv23);
SslSocket::SslSocket(boost::asio::io_service & ioService)
    : Socket(ioService, ClientContext)
{}

SslSocket::~SslSocket()
{
    Close();
}

bool SslSocket::IsSuccess(const boost::system::error_code& err) const
{
    return Socket::IsSuccess(err) || err == boost::system::error_code(ERR_PACK(ERR_LIB_SSL, 0, SSL_R_SHORT_READ), boost::asio::error::get_ssl_category());
}

void SslSocket::SetTimeout(std::size_t seconds)
{
#ifdef _WIN32
    setsockopt(m_socket.lowest_layer().native(), SOL_SOCKET, SO_RCVTIMEO, (const char*)&seconds, sizeof(seconds));
    setsockopt(m_socket.lowest_layer().native(), SOL_SOCKET, SO_SNDTIMEO, (const char*)&seconds, sizeof(seconds));
#else
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    setsockopt(m_socket.lowest_layer().native(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(m_socket.lowest_layer().native(), SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#endif
}

void SslSocket::AsyncConnect(const std::string & address, std::uint16_t port, connect_cb cb)
{
    boost::system::error_code error;
    boost::asio::ip::tcp::resolver resolver(m_socket.get_io_service());
    boost::asio::ip::tcp::resolver::query query(address, boost::lexical_cast<std::string>(port));
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query, error);
    if (!error)
    {
        auto self = shared_from_this();
        boost::asio::async_connect(m_socket.lowest_layer(), endpoint_iterator, [self, this, cb](const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::iterator it) {
            if (ec) {
                cb(ec);
            }

            m_socket.set_verify_mode(boost::asio::ssl::verify_peer);
            m_socket.set_verify_callback([](bool preverified, boost::asio::ssl::verify_context& ctx) {
                return true; // allow connections even if cert is not valid
            });

            m_socket.async_handshake(boost::asio::ssl::stream_base::client, m_strand.wrap([self, this, cb](const boost::system::error_code& ec) {
                cb(ec);
            }));
        });
    }
    else
    {
        cb(error);
    }
}

bool SslSocket::Connect(const std::string& address, std::uint16_t port)
{
    boost::system::error_code error;
    boost::asio::ip::tcp::resolver resolver(m_socket.get_io_service());
    boost::asio::ip::tcp::resolver::query query(address, boost::lexical_cast<std::string>(port));
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query, error);
    if (!error)
    {
        boost::asio::connect(m_socket.lowest_layer(), endpoint_iterator, error);
        if (!error)
        {
            m_socket.set_verify_mode(boost::asio::ssl::verify_peer);
            m_socket.set_verify_callback([](bool preverified, boost::asio::ssl::verify_context& ctx) {
                return true; // allow connections even if cert is not valid
            });

            m_socket.handshake(boost::asio::ssl::stream_base::client, error);
            return !error;
        }
    }

    return false;
}

bool SslSocket::IsConnected() const
{
    return m_socket.lowest_layer().is_open();
}

void SslSocket::Close()
{
    if (IsConnected())
    {
        boost::system::error_code ec;
        m_socket.lowest_layer().close(ec);
    }
}
}
}