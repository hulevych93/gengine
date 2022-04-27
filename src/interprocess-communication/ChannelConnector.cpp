#include <interprocess-communication/ChannelConnector.h>

#if BUILD_WINDOWS
#include <interprocess-communication/pipes/Windows/NamedPipeChannel.h>
#else
#include <interprocess-communication/pipes/Unix/UnixDomainChannel.h>
#endif

#include <interprocess-communication/tcp/TCPCommunicationEngine.h>
#include <interprocess-communication/tcp/TCPChannel.h>

namespace Gengine {
namespace InterprocessCommunication {

ChannelConnector::ChannelConnector(std::uint32_t threadId, std::unique_ptr<IChannel>& impl)
    : threadId(threadId)
    , impl(impl)
{}

bool ChannelConnector::operator()(const PipeConnection& data) const
{
#if __linux__ || __APPLE__
    auto channel = std::make_unique<UnixDomainChannel>();
#elif BUILD_WINDOWS
    auto channel = std::make_unique<NamedPipeChannel>();
#endif
    auto connected = channel->Connect(PIPE_PREFIX + data.pipe);
    if (connected)
    {
        impl = std::move(channel);
    }
    return connected;
}

bool ChannelConnector::operator()(const TcpConnection& data) const
{
    auto engine = std::make_shared<TCPCommunicationEngine>(threadId);
    auto tcpChannel = std::make_unique<TCPChannel>(engine);
    auto connected = tcpChannel->Connect(data);
    if (connected)
    {
        impl = std::move(tcpChannel);
    }
    return connected;
}

}
}



