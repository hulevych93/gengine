#pragma once

#include <interprocess-communication/InterprocessCommonDefs.h>

namespace Gengine {
namespace InterprocessCommunication {
class CommunicationEngine;
class InterprocessAcceptor;
struct ServerInitializer : boost::static_visitor<bool> {
  ServerInitializer(std::shared_ptr<CommunicationEngine>& engine,
                    std::unique_ptr<InterprocessAcceptor>& acceptor,
                    std::uint32_t threadId);

  bool operator()(const PipeConnection& data) const;
  bool operator()(const TcpConnection& data) const;

 private:
  const std::uint32_t m_threadId;
  std::shared_ptr<CommunicationEngine>& m_engine;
  std::unique_ptr<InterprocessAcceptor>& m_acceptor;
};
}  // namespace InterprocessCommunication
}  // namespace Gengine
