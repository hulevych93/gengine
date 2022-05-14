#pragma once

#include <interprocess-communication/InterprocessCommonDefs.h>
#include <memory>

#include <array>
#include <boost/variant.hpp>

namespace Gengine {
namespace InterprocessCommunication {

class InterprocessServer;
class OutputParameters;
struct RequestHeader;
class IChannel;

/**
 * @brief The ChannelAgent class
 *
 * Class which implements request/responce server side protocol.
 * Channel agents are hold by the InteprocessServer class instance.
 */
class ChannelAgent final {
 public:
  /**
   * @brief ChannelAgent constuctor
   * @param[in] the data channel
   * @param the parent server
   */
  ChannelAgent(std::unique_ptr<IChannel>&& impl, InterprocessServer& server);

  /**
   * The destructor.
   */
  ~ChannelAgent();

  /**
   * @brief QueueIO for the async processing.
   * @return true if succesfull.
   */
  bool QueueIO();

  /**
   * @brief HandleIO for the async processing.
   * @param[in] bytesProcessed
   * @return true if successfull.
   */
  bool HandleIO(std::uint32_t bytesProcessed = 0);

  /**
   * @brief GetChannel
   * @return underlying channel.
   */
  IChannel& GetChannel() { return *m_impl; }

 private:
  class ListeningChannelState final {
   public:
    bool QueueIO(ChannelAgent& agent);
    bool HandleIO(ChannelAgent& agent, std::uint32_t bytesProcessed);
  };

  friend class ListeningChannelState;

  class ProcessingChannelState final {
   public:
    bool QueueIO(ChannelAgent& agent);
    bool HandleIO(ChannelAgent& agent, std::uint32_t bytesProcessed);
  };

  friend class ProcessingChannelState;

  class RespondingChannelState final {
   public:
    bool QueueIO(ChannelAgent& agent);
    bool HandleIO(ChannelAgent& agent, std::uint32_t bytesProcessed);
  };

  friend class RespondingChannelState;

 private:
  void Execute();
  void MakeResult(ResponseCodes responseCode, const OutputParameters& outputs);
  void MakeResult(ResponseCodes responseCode);

 private:
  struct BufferImpl;
  std::unique_ptr<BufferImpl> m_buffer;

 private:
  InterprocessServer& m_server;
  std::unique_ptr<IChannel> m_impl;

  using ChannelState = boost::variant<ListeningChannelState,
                                      ProcessingChannelState,
                                      RespondingChannelState>;
  std::array<ChannelState, 3> m_states = {ListeningChannelState(),
                                          ProcessingChannelState(),
                                          RespondingChannelState()};

  std::uint32_t m_currentState = 0;
};

}  // namespace InterprocessCommunication
}  // namespace Gengine
