#pragma once

#include <atomic>
#include <interprocess-communication/tcp/sockets/ISocket.h>
#include <interprocess-communication/IChannel.h>
#include <interprocess-communication/CommunicationEngine.h>

namespace Gengine {
namespace InterprocessCommunication {
class TCPCommunicationEngine;
class TCPChannel : public IChannel
{
public:
    explicit TCPChannel(const std::shared_ptr<CommunicationEngine>& engine);
    TCPChannel(const std::shared_ptr<CommunicationEngine>& engine, const std::shared_ptr<Network::ISocket>& socket);
    ~TCPChannel();

    bool Connect(const TcpConnection& data);

protected:
    void Disconnect() override;
    bool IsConnected() const override;
    bool Send(const void* data, std::uint32_t size, std::uint32_t* bytesProccessed) override;
    bool Recv(void* data, std::uint32_t size, std::uint32_t* bytesProccessed) override;
    bool SendAsync(const void* data, std::uint32_t size) override;
    bool RecvAsync(void* data, std::uint32_t size) override;

private:
    std::shared_ptr<TCPCommunicationEngine> m_engine;
    std::shared_ptr<Network::ISocket> m_socket;
};
}
}
