#pragma once

#include <memory>
#include <interprocess-communication/InterprocessCommonDefs.h>

#include <boost/variant.hpp>
#include <array>

namespace Gengine {
namespace InterprocessCommunication {

class InterprocessServer;
class OutputParameters;
struct RequestHeader;
class IChannel;

class ChannelAgent
{
public:
    ChannelAgent(std::unique_ptr<IChannel>&& impl, InterprocessServer& server);
    ~ChannelAgent();

    bool QueueIO();
    bool HandleIO(std::uint32_t bytesProcessed = 0);

    IChannel& GetChannel()
    {
        return *m_impl;
    }

private:
    class ListeningChannelState final
    {
    public:
        bool QueueIO(ChannelAgent& agent);
        bool HandleIO(ChannelAgent& agent, std::uint32_t bytesProcessed);
    };

    friend ListeningChannelState;

    class ProcessingChannelState final
    {
    public:
        bool QueueIO(ChannelAgent& agent);
        bool HandleIO(ChannelAgent& agent, std::uint32_t bytesProcessed);
    };

    friend ProcessingChannelState;

    class RespondingChannelState final
    {
    public:
        bool QueueIO(ChannelAgent& agent);
        bool HandleIO(ChannelAgent& agent, std::uint32_t bytesProcessed);
    };

    friend RespondingChannelState;

private:
    void Execute();
    void MakeResult(ResponseCodes responseCode, const std::shared_ptr<OutputParameters>& pOutputs);
    void MakeResult(ResponseCodes responseCode);

private:
    struct BufferImpl;
    std::unique_ptr<BufferImpl> m_buffer;

private:
    InterprocessServer& m_server;
    std::unique_ptr<IChannel> m_impl;

    using ChannelState = boost::variant<ListeningChannelState, ProcessingChannelState, RespondingChannelState>;
    std::array<ChannelState, 3> m_states = { ListeningChannelState(), ProcessingChannelState(), RespondingChannelState()};

    std::uint32_t m_currentState = 0;
};

}
}
