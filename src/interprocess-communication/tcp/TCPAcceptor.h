#pragma once

#include <interprocess-communication/InterprocessAcceptor.h>

namespace Gengine {
namespace InterprocessCommunication {
class TCPCommunicationEngine;
class TCPAcceptor : public InterprocessAcceptor {
 public:
  TCPAcceptor(const TcpConnection& endpoint,
              const std::shared_ptr<CommunicationEngine>& engine);
  ~TCPAcceptor();

  void AcceptConnection(connected_callback callback) override;

 private:
  std::shared_ptr<TCPCommunicationEngine> m_engine;

 private:
  struct ListeningImpl;
  std::unique_ptr<ListeningImpl> m_listeningImpl;
};
}  // namespace InterprocessCommunication
}  // namespace Gengine