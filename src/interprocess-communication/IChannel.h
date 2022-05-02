#pragma once

#include <string>
#include <interprocess-communication/InterprocessCommonDefs.h>

namespace Gengine {
namespace InterprocessCommunication {
class IChannel
{
public:
    virtual ~IChannel() = default;

    virtual void Disconnect() = 0;
    virtual bool IsConnected() const = 0;

    /*
     * Synchronous operations on the channel.
     * `bytesProccessed` may be null in case it is unneeded.
     */
    virtual bool Send(const void* data, std::uint32_t size, std::uint32_t* bytesProccessed = nullptr) = 0;
    virtual bool Recv(void* data, std::uint32_t size, std::uint32_t* bytesProccessed = nullptr) = 0;

    virtual bool SendAsync(const void* data, std::uint32_t size) = 0;
    virtual bool RecvAsync(void* data, std::uint32_t size) = 0;
};

std::unique_ptr<IChannel> makeChannel(const std::wstring& connectionString);

}
}
