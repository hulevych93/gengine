#include "WindowsCommunicationEngine.h"
#include "NamedPipeChannel.h"

#include <Windows.h>
#include <core/Logger.h>
#include <multithreading/Event.h>

namespace Gengine {
namespace InterprocessCommunication {

struct WindowsCommunicationEngine::CallingContext
    : public boost::static_visitor<void> {
  CallingContext(NamedPipeChannel& channel, engine_callback callback)
      : channel(channel), callback(callback) {}

  void call() { boost::apply_visitor(*this, callback); }

  void operator()(accepted_callback accepted) { accepted(); }

  void operator()(readwrite_callback processed) {
    std::uint32_t bytes = 0;
    auto result = channel.GetOverlapped(&bytes);
    processed(result, bytes, true);
  }

  NamedPipeChannel& channel;
  engine_callback callback;
};

WindowsCommunicationEngine::WindowsCommunicationEngine(std::uint32_t threadId)
    : Worker(threadId), m_eventPool(1), m_loopId(Services::InvalidTimerID) {
  m_StopEvent.Create(true, false);
  m_eventPool[0] = reinterpret_cast<HANDLE>(m_StopEvent.GetOSHandle());
}

WindowsCommunicationEngine::~WindowsCommunicationEngine() = default;

void WindowsCommunicationEngine::RegisterConnection(const IChannel& connection,
                                                    engine_callback callback) {
  auto& pipeChannel = static_cast<const NamedPipeChannel&>(connection);
  auto handle = pipeChannel.GetIOHandle();

  auto handleIter = m_clientCallbacks.find(handle);
  if (handleIter != m_clientCallbacks.end()) {
    m_clientCallbacks.at(handle)->callback = callback;
  } else {
    auto index = m_eventPool.size();
    m_eventPool.resize(index + 1);

    m_eventPool.at(index) = handle;
    m_clientCallbacks.emplace(std::make_pair(
        handle, std::make_unique<CallingContext>(
                    const_cast<NamedPipeChannel&>(pipeChannel), callback)));
  }
}

void WindowsCommunicationEngine::UnregisterConnection(
    const IChannel& connection) {
  auto& pipeChannel = static_cast<const NamedPipeChannel&>(connection);
  auto handle = pipeChannel.GetIOHandle();
  auto handleIter = std::find(m_eventPool.begin(), m_eventPool.end(), handle);
  if (handleIter != m_eventPool.end()) {
    m_eventPool.erase(handleIter);
    m_clientCallbacks.erase(handle);
  }
}

void WindowsCommunicationEngine::StartInternal() {
  if (m_loopId == Services::InvalidTimerID) {
    auto handler = [this] { Loop(); };
    m_loopId = GENGINE_START_TIMER(handler, 0);
  }
}

void WindowsCommunicationEngine::StopInternal() {
  if (m_loopId != Services::InvalidTimerID) {
    m_StopEvent.Set();
    GENGINE_STOP_TIMER_WITH_WAIT(m_loopId);
    m_loopId = Services::InvalidTimerID;
  }
  Dispose();
}

void WindowsCommunicationEngine::Loop() {
  auto waitStatus = Multithreading::Event::WaitForEventsEx(&m_eventPool[0],
                                                           m_eventPool.size());
  switch (waitStatus) {
    case WAIT_OBJECT_0:
      break;
    case WAIT_FAILED:
      GLOG_ERROR("Wait events failed! Error: %d", GetLastError());
      break;
    case WAIT_TIMEOUT:
      GLOG_ERROR("Timeout in infinite mode? Is it possible?");
      break;
    default: {
      auto handle = m_eventPool.at(waitStatus);
      m_clientCallbacks.at(handle)->call();
    } break;
  }
}

std::unique_ptr<CommunicationEngine> makeEngine(std::uint32_t threadId) {
  return std::make_unique<WindowsCommunicationEngine>(threadId);
}

}  // namespace InterprocessCommunication
}  // namespace Gengine