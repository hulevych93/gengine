#include <interprocess-communication/ChannelConnector.h>

#include <interprocess-communication/IChannel.h>

#include <interprocess-communication/tcp/TCPChannel.h>
#include <interprocess-communication/tcp/TCPCommunicationEngine.h>

namespace Gengine {
namespace InterprocessCommunication {

ChannelConnector::ChannelConnector(std::uint32_t threadId,
                                   std::unique_ptr<IChannel>& impl)
    : threadId(threadId), impl(impl) {}

bool ChannelConnector::operator()(const PipeConnection& data) const {
  auto channel = makeChannel(std::wstring{ChannelAddressPrefix} + data.pipe);
  if (channel) {
    impl = std::move(channel);
    return true;
  }
  return false;
}

bool ChannelConnector::operator()(const TcpConnection& data) const {
  auto engine = std::make_shared<TCPCommunicationEngine>(threadId);
  auto tcpChannel = std::make_unique<TCPChannel>(engine);
  auto connected = tcpChannel->Connect(data);
  if (connected) {
    impl = std::move(tcpChannel);
  }
  return connected;
}

}  // namespace InterprocessCommunication
}  // namespace Gengine
