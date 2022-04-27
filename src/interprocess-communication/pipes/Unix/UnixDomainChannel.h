#pragma once

#include <atomic>
#include <interprocess-communication/IChannel.h>

namespace Gengine {
namespace InterprocessCommunication {
class UnixSocketEngine;
class UnixDomainChannel final : public IChannel
{
public:
    UnixDomainChannel();
    UnixDomainChannel(const std::shared_ptr<UnixSocketEngine>& engine);
    UnixDomainChannel(RPC_FILE_HANDLE handle, const std::shared_ptr<UnixSocketEngine>& engine);

    UnixDomainChannel(const UnixDomainChannel&) = delete;
    UnixDomainChannel(UnixDomainChannel&&) = delete;

    ~UnixDomainChannel();

    bool Connect(const std::wstring &serverSocketFileName);

    RPC_FILE_HANDLE getHandle() const
    {
        return m_socket;
    }

protected:
    void Disconnect() override;
    bool IsConnected() const override;

    bool Send(const void* data, std::uint32_t size, std::uint32_t* bytesProccessed) override;
    bool Recv(void* data, std::uint32_t size, std::uint32_t* bytesProccessed) override;

    bool SendAsync(const void* data, std::uint32_t size) override;
    bool RecvAsync(void* data, std::uint32_t size) override;

private:
    RPC_FILE_HANDLE m_socket;
    std::atomic<bool> m_stopped;
    std::shared_ptr<UnixSocketEngine> m_engine;
};
}
}
