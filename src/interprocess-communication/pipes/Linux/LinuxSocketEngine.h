#pragma once

#include <unordered_map>

#include <brokers/WorkerBroker.h>
#include <interprocess-communication/pipes/Unix/UnixSocketEngine.h>

namespace Gengine {
namespace InterprocessCommunication {

class LinuxSocketEngine : public UnixSocketEngine, public Services::Worker {
 public:
  enum class Mode : std::uint8_t { Read, Write };

 public:
  LinuxSocketEngine(std::uint32_t threadId);
  ~LinuxSocketEngine();

  LinuxSocketEngine(const UnixSocketEngine&) = delete;
  LinuxSocketEngine(UnixSocketEngine&&) = delete;

  void RegisterConnection(const IChannel& connection,
                          engine_callback callback) override;
  void UnregisterConnection(const IChannel& connection) override;

  void PostRead(const IChannel& /*connection*/,
                void* /*data*/,
                std::uint32_t /*size*/) override;
  void PostWrite(const IChannel& /*connection*/,
                 const void* /*data*/,
                 std::uint32_t /*size*/) override;

 private:
  void Post(Mode mode,
            const IChannel& connection,
            void* data,
            std::uint32_t size);

 private:
  void StartInternal() override;
  void StopInternal() override;
  void Loop();

 private:
  HandleType m_queue;
  HandleType m_stopSignal;
  HandleType m_stopSignalTrigger;

  struct ContextImpl;
  using TCallbacks =
      std::unordered_map<HandleType, std::unique_ptr<ContextImpl>>;
  TCallbacks m_clientCallbacks;

  std::uint32_t m_loopId;
};

}  // namespace InterprocessCommunication
}  // namespace Gengine
