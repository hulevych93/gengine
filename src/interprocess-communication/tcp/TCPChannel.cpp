#include "TCPChannel.h"
#include "TCPCommunicationEngine.h"

#include <interprocess-communication/tcp/sockets/Socket.h>
#include <core/Logger.h>

namespace Gengine {
using namespace Network;
namespace InterprocessCommunication {

TCPChannel::TCPChannel(const std::shared_ptr<CommunicationEngine>& engine)
    : m_engine(std::static_pointer_cast<TCPCommunicationEngine>(engine))
{}

TCPChannel::TCPChannel(const std::shared_ptr<CommunicationEngine>& engine, const std::shared_ptr<Network::ISocket>& socket)
    : m_engine(std::static_pointer_cast<TCPCommunicationEngine>(engine))
    , m_socket(socket)
{}

TCPChannel::~TCPChannel()
{
    if (m_socket)
    {
        m_socket->Close();
    }
}

bool TCPChannel::Connect(const TcpConnection& data)
{
    if (!IsConnected())
    {
        if (!m_socket)
        {
            m_socket = m_engine->Create(false);
        }
        return m_socket->Connect(data.ip, data.port);
    }
    return true;
}

void TCPChannel::Disconnect()
{
    if (IsConnected())
    {
        m_socket->Close();
    }
}

bool TCPChannel::IsConnected() const
{
    return m_socket ? m_socket->IsConnected() : false;
}

bool TCPChannel::Send(const void* data, std::uint32_t size, std::uint32_t* bytesProccessed)
{
    if (IsConnected())
    {
        auto result = m_socket->Write(boost::asio::buffer(data, size));
        if(bytesProccessed)
            *bytesProccessed = result;
        return result > 0;
    }

    return false;
}

bool TCPChannel::Recv(void* data, std::uint32_t size, std::uint32_t* bytesProccessed)
{
    if (IsConnected())
    {
        boost::asio::streambuf buff(size);
        auto processed = m_socket->Read(buff, size);
        memcpy(data, buff.data().data(), size);
        if(bytesProccessed)
            *bytesProccessed = processed;
        return processed > 0;
    }

    return false;
}

bool TCPChannel::SendAsync(const void* data, std::uint32_t size)
{
    if (IsConnected())
    {
        auto callback = boost::get<readwrite_callback>(m_engine->GetCallback(*this));
        m_socket->AsyncWrite(boost::asio::buffer(data, size), [callback](const boost::system::error_code& ec, size_t processed) {
            callback(ec.value() == 0, processed, true);
        });
        return true;
    }

    return false;
}

bool TCPChannel::RecvAsync(void* data, std::uint32_t size)
{
    if (IsConnected())
    {
        auto callback = boost::get<readwrite_callback>(m_engine->GetCallback(*this));
        auto buff = std::make_shared<boost::asio::streambuf>(size);
        m_socket->AsyncReadAtLeast(*buff, [engine = m_engine, callback, buff, data](const boost::system::error_code& ec, size_t processed) {
            auto success = ec.value() == 0;
            if (success)
            {
                memcpy(data, buff->data().data(), processed);
            }
            callback(success, processed, true);
        }, size);
        return true;
    }

    return false;
}

}
}
