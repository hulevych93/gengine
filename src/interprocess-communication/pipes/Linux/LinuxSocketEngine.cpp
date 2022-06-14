#include "LinuxSocketEngine.h"

#include <core/Logger.h>
#include <interprocess-communication/InterprocessCommonDefs.h>
#include <interprocess-communication/pipes/Unix/UnixDomainChannel.h>

#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/types.h>

#include <fcntl.h>
#include <unistd.h>

namespace Gengine {
namespace InterprocessCommunication {

struct LinuxSocketEngine::ContextImpl : public boost::static_visitor<void> {
 public:
  struct pending_operation final {
    void* data = nullptr;
    size_t size = 0u;
  };

  using pending_operation_opt = boost::optional<pending_operation>;

 public:
  ContextImpl(IChannel& channel, engine_callback callback)
      : channel(channel), callback(callback) {}

  void operator()(accepted_callback accepted) { accepted(); }

  void operator()(readwrite_callback processed) {
    assert(pending.has_value());

    pending_operation op = pending.get();
    pending = pending_operation_opt{};

    std::uint32_t bytesProcessed = 0;
    auto error = false;
    switch (_mode) {
      case LinuxSocketEngine::Mode::Read:
        error = channel.Recv(op.data, op.size, &bytesProcessed);
        break;
      case LinuxSocketEngine::Mode::Write:
        error = channel.Send(op.data, op.size, &bytesProcessed);
        break;
    };
    processed(error, bytesProcessed, true);
  }

  void call(const LinuxSocketEngine::Mode mode) {
    _mode = mode;
    boost::apply_visitor(*this, callback);
  }

  LinuxSocketEngine::Mode _mode = Mode::Write;
  IChannel& channel;
  pending_operation_opt pending;
  engine_callback callback;
};

LinuxSocketEngine::LinuxSocketEngine(std::uint32_t threadId)
    : Worker(threadId), m_loopId(Services::InvalidTimerID) {
  m_queue = epoll_create1(0);
  assert(m_queue >= 0);

  int pipe_fds[2];
  assert(pipe(pipe_fds) == 0);

  m_stopSignal = pipe_fds[0];
  assert(::fcntl(m_stopSignal, F_SETFL, O_NONBLOCK) >= 0);
  m_stopSignalTrigger = pipe_fds[1];
  assert(::fcntl(m_stopSignalTrigger, F_SETFL, O_NONBLOCK) >= 0);

  struct epoll_event event;
  event.data.fd = m_stopSignal;
  event.events = EPOLLOUT;
  assert(epoll_ctl(m_queue, EPOLL_CTL_ADD, m_stopSignal, &event) >= 0);
}

LinuxSocketEngine::~LinuxSocketEngine() {
  if (m_stopSignal != InvalidHandle) {
    ::close(m_stopSignal);
  }
  if (m_stopSignalTrigger != InvalidHandle) {
    ::close(m_stopSignalTrigger);
  }

  ::close(m_queue);
}

void LinuxSocketEngine::RegisterConnection(const IChannel& connection,
                                           engine_callback callback) {
  auto& unixSocket = static_cast<const UnixDomainChannel&>(connection);
  auto handle = unixSocket.getHandle();

  assert(m_clientCallbacks.find(handle) == m_clientCallbacks.end());

  auto& context = m_clientCallbacks[handle];
  context = std::make_unique<ContextImpl>(
      const_cast<UnixDomainChannel&>(unixSocket), callback);

  struct epoll_event events;
  events.events = EPOLLIN | EPOLLOUT;
  events.data.ptr = context.get();

  assert(epoll_ctl(m_queue, EPOLL_CTL_ADD, handle, &events) >= 0);
}

void LinuxSocketEngine::PostWrite(const IChannel& connection,
                                  const void* data,
                                  std::uint32_t size) {
  Post(Mode::Write, connection, const_cast<void*>(data), size);
}

void LinuxSocketEngine::PostRead(const IChannel& connection,
                                 void* data,
                                 std::uint32_t size) {
  Post(Mode::Read, connection, data, size);
}

void LinuxSocketEngine::Post(const Mode mode,
                             const IChannel& connection,
                             void* data,
                             std::uint32_t size) {
  auto& unixSocket = static_cast<const UnixDomainChannel&>(connection);
  auto handle = unixSocket.getHandle();

  auto iter = m_clientCallbacks.find(handle);
  assert(iter != m_clientCallbacks.end());

  auto& context = iter->second;
  assert(!context->pending.has_value());
  context->pending = ContextImpl::pending_operation{data, size};

  struct epoll_event event;
  event.events = mode == Mode::Read ? EPOLLOUT : EPOLLIN;
  event.data.ptr = context.get();

  assert(epoll_ctl(m_queue, EPOLL_CTL_DEL, handle, &event) >= 0);
  event.events = mode == Mode::Read ? EPOLLIN : EPOLLOUT;
  assert(epoll_ctl(m_queue, EPOLL_CTL_ADD, handle, &event) >= 0);
}

void LinuxSocketEngine::UnregisterConnection(const IChannel& connection) {
  auto& pipeChannel = static_cast<const UnixDomainChannel&>(connection);
  auto handle = pipeChannel.getHandle();

  auto handleIter = m_clientCallbacks.find(handle);
  if (handleIter != m_clientCallbacks.end()) {
    struct epoll_event event;
    event.events = EPOLLIN;
    assert(epoll_ctl(m_queue, EPOLL_CTL_DEL, handle, &event) >= 0);

    m_clientCallbacks.erase(handle);
  }
}

void LinuxSocketEngine::StartInternal() {
  if (m_loopId == Services::InvalidTimerID) {
    auto handler = [this] { Loop(); };
    m_loopId = GENGINE_START_LOOP(handler);
  }
}

void LinuxSocketEngine::StopInternal() {
  if (m_loopId != Services::InvalidTimerID) {
    char byte = 0;
    assert(write(m_stopSignalTrigger, &byte, 1) != -1 || errno == EWOULDBLOCK);

    GENGINE_STOP_TIMER_WITH_WAIT(m_loopId);
    m_loopId = Services::InvalidTimerID;
  }
  Dispose();
}

void LinuxSocketEngine::Loop() {
  static struct epoll_event evList[5];

  auto nev = epoll_wait(m_queue, evList, 5, -1);
  if (nev >= 1) {
    for (int i = 0; i < nev; i++) {
      const auto ident = evList[i].data.fd;
      if (ident == m_stopSignal) {
        GLOG_INFO("Stop signal triggered.");
        break;
      }

      if (evList[i].events & EPOLLERR) {
        struct epoll_event event;
        event.events = EPOLLIN;
        assert(epoll_ctl(m_queue, EPOLL_CTL_DEL, 0, &event) >= 0);
      }

      if (evList[i].events & EPOLLIN || evList[i].events & EPOLLOUT) {
        auto* context = reinterpret_cast<ContextImpl*>(evList[i].data.ptr);
        context->call(evList[i].events & EPOLLIN ? Mode::Read : Mode::Write);
      }
    }
  }
}

std::unique_ptr<CommunicationEngine> makeEngine(std::uint32_t threadId) {
  return std::make_unique<LinuxSocketEngine>(threadId);
}

}  // namespace InterprocessCommunication
}  // namespace Gengine
