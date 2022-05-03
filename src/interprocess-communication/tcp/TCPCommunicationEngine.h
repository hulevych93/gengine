#pragma once

#include <brokers/WorkerBroker.h>

#include <interprocess-communication/CommunicationEngine.h>
#include <interprocess-communication/tcp/sockets/ISocket.h>

namespace Gengine {
namespace InterprocessCommunication {
class TCPCommunicationEngine : public CommunicationEngine,
                               public Services::Worker {
 public:
  TCPCommunicationEngine(std::uint32_t threadId);
  ~TCPCommunicationEngine();

  void RegisterConnection(const IChannel& connection,
                          engine_callback callback) override;
  void UnregisterConnection(const IChannel& connection) override;

  std::shared_ptr<Network::ISocket> Create(bool secured);
  engine_callback GetCallback(const IChannel& connection) const;

  void* GetInternal() const;

 protected:
  void StartInternal() override;
  void StopInternal() override;

 private:
  struct EngineImpl;
  std::unique_ptr<EngineImpl> m_engineImpl;
};
}  // namespace InterprocessCommunication
}  // namespace Gengine
