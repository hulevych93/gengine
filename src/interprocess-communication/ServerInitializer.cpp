#include <interprocess-communication/ServerInitializer.h>

#if defined(_WIN32)
#include <interprocess-communication/pipes/Windows/WindowsCommunicationEngine.h>
#include <interprocess-communication/pipes/Windows/NamedPipeAcceptor.h>
#elif __APPLE__ || __linux__
#include <interprocess-communication/pipes/Unix/UnixSocketAcceptor.h>
#endif
#if __APPLE__
#include <interprocess-communication/pipes/MacOS/UnixSocketEngine.h>
#endif

#include <interprocess-communication/tcp/TCPAcceptor.h>
#include <interprocess-communication/tcp/TCPCommunicationEngine.h>
#include <core/Logger.h>

namespace Gengine {
namespace InterprocessCommunication {

ServerInitializer::ServerInitializer(std::shared_ptr<CommunicationEngine>& engine, std::unique_ptr<InterprocessAcceptor>& acceptor, std::uint32_t threadId)
   : m_threadId(threadId)
   , m_engine(engine)
   , m_acceptor(acceptor)
{}

bool ServerInitializer::operator()(const PipeConnection& data) const
{
    GLOG_INFO("Starting namep pipe acceptor on pipe: %s", data);
#if defined(_WIN32)
    m_engine = std::make_shared<WindowsCommunicationEngine>(m_threadId);
    m_acceptor = std::make_unique<NamedPipeAcceptor>(PIPE_PREFIX + data, m_engine);
    return true;
#elif __linux__ || __APPLE__
    m_engine = std::make_shared<UnixSocketEngine>(m_threadId);
    m_acceptor = std::make_unique<UnixSocketAcceptor>(PIPE_PREFIX + data.pipe, m_engine);
    return true;
#endif

    return false;
}

bool ServerInitializer::operator()(const TcpConnection& data) const
{
   GLOG_INFO("Starting tcp acceptor on: %s, %d", data.ip, data.port);
   m_engine = std::make_shared<TCPCommunicationEngine>(m_threadId);
   m_acceptor = std::make_unique<TCPAcceptor>(data, m_engine);
   return true;
}

}
}



