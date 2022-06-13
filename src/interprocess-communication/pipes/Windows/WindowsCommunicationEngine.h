#pragma once

#include <brokers/WorkerBroker.h>
#include <interprocess-communication/CommunicationEngine.h>
#include <unordered_map>

namespace Gengine {
namespace InterprocessCommunication {
class WindowsCommunicationEngine : public CommunicationEngine,
                                   public Services::Worker {
 public:
  WindowsCommunicationEngine(std::uint32_t threadId);
  ~WindowsCommunicationEngine();
  void RegisterConnection(const IChannel& connection,
                          engine_callback callback) override;
  void UnregisterConnection(const IChannel& connection) override;

 protected:
  void StartInternal() override;
  void StopInternal() override;
  void Loop();

 private:
  void* m_stopEvent;

  using TWaitEvents = std::vector<void*>;
  TWaitEvents m_eventPool;

  struct CallingContext;
  using TCallbacks = std::unordered_map<void*, std::unique_ptr<CallingContext>>;
  TCallbacks m_clientCallbacks;

  std::uint32_t m_loopId;
};
}  // namespace InterprocessCommunication
}  // namespace Gengine
