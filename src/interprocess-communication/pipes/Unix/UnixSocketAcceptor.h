#pragma once

#include <interprocess-communication/InterprocessAcceptor.h>
#include <atomic>
#include <vector>

namespace Gengine {
namespace InterprocessCommunication {
class UnixSocketAcceptor final : public InterprocessAcceptor {
 public:
  UnixSocketAcceptor(const std::wstring& socketFileName,
                     const std::shared_ptr<CommunicationEngine>& engine);
  ~UnixSocketAcceptor();

  void AcceptConnection(connected_callback callback) override;

 private:
  std::shared_ptr<CommunicationEngine> m_engine;
  std::unique_ptr<IChannel> m_channel;

 private:
  const std::wstring m_socketFileName;
};
}  // namespace InterprocessCommunication
}  // namespace Gengine
