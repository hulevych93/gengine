#include "UnixSocketEngine.h"

#include <interprocess-communication/pipes/Unix/UnixDomainChannel.h>
#include <interprocess-communication/InterprocessCommonDefs.h>
#include <core/Logger.h>

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

#include <fcntl.h>
#include <unistd.h>

namespace Gengine {
namespace InterprocessCommunication {

struct UnixSocketEngine::ContextImpl: public boost::static_visitor<void>
{
public:
    struct pending_operation final
    {
        void* data = nullptr;
        size_t size = 0u;
    };

    using pending_operation_opt = boost::optional<pending_operation>;

public:
    ContextImpl(IChannel& channel, engine_callback callback)
        : channel(channel)
        , callback(callback)
    {}

    void operator()(accepted_callback accepted)
    {
        accepted();
    }

    void operator()(readwrite_callback processed)
    {
        assert(pending.has_value());

        pending_operation op = pending.get();
        pending = pending_operation_opt{};

        std::uint32_t bytesProcessed = 0;
        auto error = false;
        switch(_mode)
        {
        case UnixSocketEngine::Mode::Read:
            error = channel.Recv(op.data, op.size, &bytesProcessed);
            break;
        case UnixSocketEngine::Mode::Write:
            error = channel.Send(op.data, op.size, &bytesProcessed);
            break;
        };
        processed(error, bytesProcessed, true);
    }

    void call(const UnixSocketEngine::Mode mode)
    {
        _mode = mode;
        boost::apply_visitor(*this, callback);
    }

    UnixSocketEngine::Mode _mode = Mode::Write;
    IChannel& channel;
    pending_operation_opt pending;
    engine_callback callback;
};

UnixSocketEngine::UnixSocketEngine(std::uint32_t threadId)
: Worker(threadId)
, m_loopId(Services::INVALID_TIMER_ID)
, m_queue(kqueue())
{
    int pipe_fds[2];
    assert(pipe(pipe_fds) == 0);

    m_stopSignal = pipe_fds[0];
    ::fcntl(m_stopSignal, F_SETFL, O_NONBLOCK);
    m_stopSignalTrigger = pipe_fds[1];
    ::fcntl(m_stopSignalTrigger, F_SETFL, O_NONBLOCK);

    struct kevent events[1];
    EV_SET(&events[0], m_stopSignal, EVFILT_READ, EV_ADD, 0, 0, 0);
    assert(kevent(m_queue, events, 1, 0, 0, 0) != -1);
}

UnixSocketEngine::~UnixSocketEngine()
{
    if (m_stopSignal != InvalidHandle)
    {
        ::close(m_stopSignal);
    }
    if (m_stopSignalTrigger != InvalidHandle)
    {
        ::close(m_stopSignalTrigger);
    }
}

void UnixSocketEngine::RegisterConnection(const IChannel& connection, engine_callback callback)
{
    auto& unixSocket = static_cast<const UnixDomainChannel&>(connection);
    auto handle = unixSocket.getHandle();

    assert(m_clientCallbacks.find(handle) == m_clientCallbacks.end());

    auto& context = m_clientCallbacks[handle];
    context = std::make_unique<ContextImpl>(const_cast<UnixDomainChannel&>(unixSocket), callback);

    struct kevent events[2];
    EV_SET(&events[0], handle, EVFILT_READ, EV_ADD, 0, 0, &(*context));
    EV_SET(&events[1], handle, EVFILT_WRITE, EV_ADD, 0, 0, &(*context));
    assert(::kevent(m_queue, events, 2, NULL, 0, NULL) != -1);
}

void UnixSocketEngine::PostWrite(const IChannel& connection, const void* data, std::uint32_t size)
{
    Post(Mode::Write, connection, const_cast<void*>(data), size);
}

void UnixSocketEngine::PostRead(const IChannel& connection, void* data, std::uint32_t size)
{
    Post(Mode::Read, connection, data, size);
}

void UnixSocketEngine::Post(const Mode mode, const IChannel& connection, void* data, std::uint32_t size)
{
    auto& unixSocket = static_cast<const UnixDomainChannel&>(connection);
    auto handle = unixSocket.getHandle();

    auto iter = m_clientCallbacks.find(handle);
    assert(iter != m_clientCallbacks.end());

    auto& context = iter->second;
    assert(!context->pending.has_value());
    context->pending = ContextImpl::pending_operation{data, size};

    struct kevent events[1];
    EV_SET(&events[0], handle, mode == Mode::Read ? EVFILT_WRITE : EVFILT_READ , EV_DELETE, 0, 0, &(*context));
    ::kevent(m_queue, events, 1, NULL, 0, NULL);

    EV_SET(&events[0], handle, mode == Mode::Read ? EVFILT_READ : EVFILT_WRITE, EV_ADD, 0, 0, &(*context));
    ::kevent(m_queue, events, 1, NULL, 0, NULL);
}

void UnixSocketEngine::UnregisterConnection(const IChannel& connection)
{
    auto& pipeChannel = static_cast<const UnixDomainChannel&>(connection);
    auto handle = pipeChannel.getHandle();

    auto handleIter = m_clientCallbacks.find(handle);
    if (handleIter != m_clientCallbacks.end())
    {
        struct kevent events[1];
        EV_SET(&events[0], handle, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
        assert(::kevent(m_queue, events, 1, 0, 0, 0) != -1);

        m_clientCallbacks.erase(handle);
    }
}

void UnixSocketEngine::StartInternal()
{
    if (m_loopId == Services::INVALID_TIMER_ID)
    {
        auto handler = [this] { Loop(); };
        m_loopId = START_HEARTBEAT_TIMER(handler, 0);
    }
}

void UnixSocketEngine::StopInternal()
{
    if (m_loopId != Services::INVALID_TIMER_ID)
    {
        char byte = 0;
        assert(write(m_stopSignalTrigger, &byte, 1) != -1 || errno == EWOULDBLOCK);

        STOP_HEARTBEAT_TIMER_WITH_WAIT(m_loopId);
        m_loopId = Services::INVALID_TIMER_ID;
    }
    Dispose();
}

void UnixSocketEngine::Loop()
{
    static struct kevent evList[32];

    auto nev = kevent(m_queue, NULL, 0, evList, 32, NULL);
    if (nev >= 1)
    {
        for (int i = 0; i < nev; i++)
        {
            const auto ident = evList[i].ident;
            if(ident == m_stopSignal)
            {
                GLOG_INFO("Stop signal triggered.");
                break;
            }

            if (evList[i].flags & EV_EOF)
            {
                struct kevent event;
                EV_SET(&event, ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);
                ::kevent(m_queue, &event, 1, NULL, 0, NULL);
            }
            else
            {
                auto* context = reinterpret_cast<ContextImpl*>(evList[i].udata);
                context->call(evList[i].filter == EVFILT_READ ? Mode::Read : Mode::Write);
            }
        }
    }
}

std::unique_ptr<CommunicationEngine> makeEngine(std::uint32_t threadId)
{
    return std::make_unique<UnixSocketEngine>(threadId);
}

}
}
