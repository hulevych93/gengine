#pragma once

#include <atomic>
#include <interprocess-communication/IChannel.h>
#include <multithreading/Event.h>

namespace Gengine {
namespace InterprocessCommunication {
class NamedPipeChannel : public IChannel
{
public:
    NamedPipeChannel();
    NamedPipeChannel(RPC_FILE_HANDLE handle);
    ~NamedPipeChannel();

    bool Connect(const std::wstring& data);
    void* GetIOHandle() const;
    bool GetOverlapped(std::uint32_t *uiBytesTransferred);

protected:
    void Disconnect() override;
    bool IsConnected() const override;
    bool Send(const void* data, std::uint32_t size, std::uint32_t* bytesProccessed = nullptr) override;
    bool Recv(void* data, std::uint32_t size, std::uint32_t* bytesProccessed = nullptr) override;
    bool SendAsync(const void* data, std::uint32_t size) override;
    bool RecvAsync(void* data, std::uint32_t size) override;

private:
    RPC_FILE_HANDLE m_socket;
    std::atomic<bool> m_stopped;
    mutable Multithreading::Event m_ioReady;
    Multithreading::Event m_evtStopped;
    std::vector<void*> m_changeHandles;
    void* m_overlapped;
};
}
}