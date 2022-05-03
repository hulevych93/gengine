#include "ChannelAgent.h"

#include <interprocess-communication/CommunicationEngine.h>
#include <interprocess-communication/IChannel.h>
#include <interprocess-communication/InputParameters.h>
#include <interprocess-communication/InterprocessServer.h>
#include <interprocess-communication/OutputParameters.h>

#include <core/Logger.h>

namespace Gengine {
namespace InterprocessCommunication {

struct ChannelAgent::BufferImpl {
  BufferImpl()
      : data_(nullptr),
        requestHeader_(std::make_unique<RequestHeader>()),
        size(0),
        processed(0) {}

  void allocate(std::uint32_t bytes) {
    if (size < bytes) {
      size = bytes;
      data_ = std::make_unique<std::uint8_t[]>(size);
    }
    processed = 0;
  }

  std::uint8_t* requestHeaderData() {
    return reinterpret_cast<std::uint8_t*>(requestHeader_.get());
  }

  std::uint8_t* data() { return reinterpret_cast<std::uint8_t*>(data_.get()); }

  bool isReady() const { return requestHeader_->requestDataSize <= 0; }

  std::unique_ptr<std::uint8_t[]> data_;
  std::unique_ptr<RequestHeader> requestHeader_;
  std::uint32_t size;
  std::uint32_t processed;
};

struct QueueVisitor final : boost::static_visitor<bool> {
  QueueVisitor(ChannelAgent& agent) : agent(agent) {}

  template <typename Type>
  bool operator()(Type& state) {
    return state.QueueIO(agent);
  }

  ChannelAgent& agent;
};

struct HandleVisitor final : boost::static_visitor<bool> {
  HandleVisitor(ChannelAgent& agent, std::uint32_t bytesProcessed)
      : agent(agent), bytesProcessed(bytesProcessed) {}

  template <typename Type>
  bool operator()(Type& state) {
    return state.HandleIO(agent, bytesProcessed);
  }

  ChannelAgent& agent;
  std::uint32_t bytesProcessed = 0;
};

ChannelAgent::ChannelAgent(std::unique_ptr<IChannel>&& impl,
                           InterprocessServer& server)
    : m_buffer(std::make_unique<BufferImpl>()),
      m_server(server),
      m_impl(std::move(impl)) {}

ChannelAgent::~ChannelAgent() {}

bool ChannelAgent::QueueIO() {
  QueueVisitor visitor{*this};
  return boost::apply_visitor(visitor, m_states[m_currentState]);
}

bool ChannelAgent::HandleIO(const std::uint32_t bytesProcessed) {
  HandleVisitor visitor{*this, bytesProcessed};
  return boost::apply_visitor(visitor, m_states[m_currentState]);
}

bool ChannelAgent::ListeningChannelState::QueueIO(ChannelAgent& agent) {
  auto left = sizeof(RequestHeader) - agent.m_buffer->processed;
  assert(left > 0);
  auto buffer = agent.m_buffer->requestHeaderData();
  return agent.m_impl->RecvAsync(buffer + agent.m_buffer->processed, left);
}

bool ChannelAgent::ListeningChannelState::HandleIO(
    ChannelAgent& agent,
    std::uint32_t bytesProcessed) {
  agent.m_buffer->processed += bytesProcessed;
  auto left = sizeof(RequestHeader) - agent.m_buffer->processed;
  if (left == 0) {
    if (agent.m_buffer->isReady()) {
      agent.Execute();
      agent.m_currentState = 2u;
    } else {
      agent.m_buffer->allocate(agent.m_buffer->requestHeader_->requestDataSize);
      agent.m_currentState = 1u;
    }
  }
  return left == 0;
}

bool ChannelAgent::ProcessingChannelState::QueueIO(ChannelAgent& agent) {
  auto left = agent.m_buffer->requestHeader_->requestDataSize -
              agent.m_buffer->processed;
  assert(left > 0);
  auto buf = agent.m_buffer->data();
  return agent.m_impl->RecvAsync(buf + agent.m_buffer->processed, left);
}

bool ChannelAgent::ProcessingChannelState::HandleIO(
    ChannelAgent& agent,
    std::uint32_t bytesProcessed) {
  auto left = agent.m_buffer->requestHeader_->requestDataSize -
              agent.m_buffer->processed;
  assert(left > 0);
  agent.m_buffer->processed += bytesProcessed;
  left -= bytesProcessed;
  if (left == 0) {
    agent.Execute();
    agent.m_currentState = 2u;
  }
  return left == 0;
}

bool ChannelAgent::RespondingChannelState::QueueIO(ChannelAgent& agent) {
  auto responseHeader =
      reinterpret_cast<ResponseHeader*>(agent.m_buffer->data());
  auto left = sizeof(ResponseHeader) + responseHeader->responseDataSize -
              agent.m_buffer->processed;
  assert(left > 0);
  auto buf = agent.m_buffer->data();
  return agent.m_impl->SendAsync(buf + agent.m_buffer->processed, left);
}

bool ChannelAgent::RespondingChannelState::HandleIO(
    ChannelAgent& agent,
    std::uint32_t bytesProcessed) {
  auto responseHeader =
      reinterpret_cast<ResponseHeader*>(agent.m_buffer->data());
  auto left = sizeof(ResponseHeader) + responseHeader->responseDataSize -
              agent.m_buffer->processed;
  assert(left > 0);
  agent.m_buffer->processed += bytesProcessed;
  left -= bytesProcessed;
  if (left == 0) {
    agent.m_buffer->processed = 0;
    agent.m_currentState = 0u;
  }
  return left == 0;
}

void ChannelAgent::Execute() {
  auto inputs = std::make_shared<InputParameters>();
  if (m_buffer->requestHeader_->requestDataSize > 0) {
    if (!inputs->Deserialize(m_buffer->data(),
                             m_buffer->requestHeader_->requestDataSize)) {
      GLOG_ERROR("Failed deserialize RPC request; Function code %08X",
                 m_buffer->requestHeader_->functionCode);
      MakeResult(ResponseCodes::InvalidRequest);
      return;
    }
  }
  if (m_buffer->requestHeader_->request) {
    auto outputs = std::make_shared<OutputParameters>();
    auto result = m_server.ProcessRequest(
        m_buffer->requestHeader_->functionCode,
        m_buffer->requestHeader_->interfaceKey, inputs, outputs);
    MakeResult(result, outputs);
  } else {
    m_server.ProcessEvent(m_buffer->requestHeader_->functionCode,
                          m_buffer->requestHeader_->interfaceKey, inputs);
    MakeResult(ResponseCodes::Ok);
  }
}

void ChannelAgent::MakeResult(
    ResponseCodes result,
    const std::shared_ptr<OutputParameters>& parameters) {
  const auto size = parameters->GetSize();
  const auto sizeRequired = size + sizeof(ResponseHeader);
  m_buffer->allocate(sizeRequired);

  auto responseHeader = reinterpret_cast<ResponseHeader*>(m_buffer->data());
  responseHeader->responseDataSize = size;
  responseHeader->responseCode = result;
  if (size > 0u) {
    memcpy(m_buffer->data() + sizeof(ResponseHeader), parameters->GetData(),
           size);
  }
}

void ChannelAgent::MakeResult(ResponseCodes responseCode) {
  const auto sizeRequired = sizeof(ResponseHeader);
  m_buffer->allocate(sizeRequired);

  auto* responseHeader = reinterpret_cast<ResponseHeader*>(m_buffer->data());
  responseHeader->responseDataSize = 0;
  responseHeader->responseCode = responseCode;
}

}  // namespace InterprocessCommunication
}  // namespace Gengine
