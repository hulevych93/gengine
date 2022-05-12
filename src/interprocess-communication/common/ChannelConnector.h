#pragma once

#include <interprocess-communication/InterprocessCommonDefs.h>

namespace Gengine {
namespace InterprocessCommunication {
class IChannel;

struct ChannelConnector : boost::static_visitor<bool> {
  ChannelConnector(std::uint32_t threadId, std::unique_ptr<IChannel>& impl);

  bool operator()(const PipeConnection& data) const;
  bool operator()(const TcpConnection& data) const;

 private:
  std::unique_ptr<IChannel>& impl;
  std::uint32_t threadId;
};
}  // namespace InterprocessCommunication
}  // namespace Gengine
