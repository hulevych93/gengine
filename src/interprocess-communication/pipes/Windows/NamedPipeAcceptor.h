#pragma once

#include <interprocess-communication/InterprocessAcceptor.h>

namespace Gengine {
namespace InterprocessCommunication {
class CommunicationEngine;
class NamedPipeAcceptor : public InterprocessAcceptor {
 public:
  NamedPipeAcceptor(const std::wstring& pipe,
                    const std::shared_ptr<CommunicationEngine>& engine);
  ~NamedPipeAcceptor();

  void AcceptConnection(connected_callback callback) override;

 private:
  bool CreateListeningHandle();

 private:
  std::shared_ptr<CommunicationEngine> m_engine;
  const std::wstring m_pipe;

 private:
  struct ListeningImpl;
  std::unique_ptr<ListeningImpl> m_listeningImpl;
};
}  // namespace InterprocessCommunication
}  // namespace Gengine