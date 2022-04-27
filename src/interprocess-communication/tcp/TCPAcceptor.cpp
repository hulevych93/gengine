#include "TCPAcceptor.h"
#include "TCPChannel.h"
#include "TCPCommunicationEngine.h"

#include <interprocess-communication/tcp/sockets/TCPSocket.h>
#include <core/Logger.h>

namespace bai = boost::asio::ip;

namespace Gengine {
namespace InterprocessCommunication {
using namespace Network;

struct TCPAcceptor::ListeningImpl
{
    static std::unique_ptr<ListeningImpl> Create(boost::asio::io_context& service, const TcpConnection& tcp_endpoint)
    {
        auto impl = std::make_unique<ListeningImpl>(service);
        try
        {
            auto endpoint = bai::tcp::endpoint(boost::asio::ip::address::from_string(tcp_endpoint.ip), tcp_endpoint.port);
            impl->acceptor.open(endpoint.protocol());
            impl->acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
            impl->acceptor.bind(endpoint);
            impl->acceptor.listen();
            return impl;
        }
        catch (std::exception& ex)
        {
            GLOG_WARNING_INTERNAL("Failed to bind endpoint: %s:%d, what: %s",
                tcp_endpoint.ip,
                tcp_endpoint.port,
                ex.what());
        }
        return std::unique_ptr<ListeningImpl>();
    }

    ListeningImpl(boost::asio::io_context& service)
        : acceptor(service)
    {}

    boost::asio::ip::tcp::acceptor acceptor;
};

TCPAcceptor::TCPAcceptor(const TcpConnection& endpoint, const std::shared_ptr<CommunicationEngine>& engine)
    : m_engine(std::static_pointer_cast<TCPCommunicationEngine>(engine))
    , m_listeningImpl(ListeningImpl::Create(*reinterpret_cast<boost::asio::io_context*>(m_engine->GetInternal()), endpoint))
{}

TCPAcceptor::~TCPAcceptor() = default;

void TCPAcceptor::AcceptConnection(connected_callback callback)
{
    if (m_listeningImpl)
    {
        auto socket = m_engine->Create(false);
        m_listeningImpl->acceptor.async_accept(static_cast<TCPSocket&>(*socket).m_socket, [self = this, socket, callback](const boost::system::error_code& ec) {
            callback(ec.value() == 0, std::make_unique<TCPChannel>(self->m_engine, socket));
        });
    }
}

}
}
