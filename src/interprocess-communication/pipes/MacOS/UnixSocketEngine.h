#pragma once

#include <unordered_map>

#include <brokers/WorkerBroker.h>
#include <interprocess-communication/CommunicationEngine.h>

namespace Gengine {
namespace InterprocessCommunication {

class UnixSocketEngine final : public CommunicationEngine,
    public Services::Worker
{
public:
    enum class Mode : std::uint8_t
    {
        Read,
        Write
    };

public:
    UnixSocketEngine(std::uint32_t threadId);
    ~UnixSocketEngine();

    UnixSocketEngine(const UnixSocketEngine&) = delete;
    UnixSocketEngine(UnixSocketEngine&&) = delete;

    void RegisterConnection(const IChannel& connection, engine_callback callback) override;
    void UnregisterConnection(const IChannel& connection) override;

    void PostRead(const IChannel& connection, void* data, std::uint32_t size);
    void PostWrite(const IChannel& connection, const void* data, std::uint32_t size);

private:
    void Post(Mode mode, const IChannel& connection, void* data, std::uint32_t size);

private:
    void StartInternal() override;
    void StopInternal() override;
    void Loop();

private:
    HandleType m_queue;
    HandleType m_stopSignal;
    HandleType m_stopSignalTrigger;

    struct ContextImpl;
    using TCallbacks = std::unordered_map<HandleType, std::unique_ptr<ContextImpl>>;
    TCallbacks m_clientCallbacks;

    std::uint32_t m_loopId;
};
}
}

