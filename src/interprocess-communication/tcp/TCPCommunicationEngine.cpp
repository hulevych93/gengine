#include "TCPCommunicationEngine.h"
#include "TCPChannel.h"

#include <boost/asio.hpp>

#include <interprocess-communication/tcp/sockets/TCPSocket.h>
#include <core/Logger.h>

namespace Gengine {
namespace InterprocessCommunication {
using namespace Services;
using namespace Network;

struct TCPCommunicationEngine::EngineImpl
{
    EngineImpl()
        : service()
        , work()
    {}
    boost::asio::io_context service;
    std::unique_ptr<boost::asio::io_context::work> work;

    using TCallbacks = std::unordered_map<IChannel*, engine_callback>;
    TCallbacks callbacks;
};

TCPCommunicationEngine::TCPCommunicationEngine(std::uint32_t threadId)
: Worker(threadId)
, m_engineImpl(std::make_unique<EngineImpl>())
{}

TCPCommunicationEngine::~TCPCommunicationEngine() = default;

void TCPCommunicationEngine::RegisterConnection(const IChannel& connection, engine_callback callback)
{
    auto key = const_cast<IChannel*>(&connection);
    auto handleIter = m_engineImpl->callbacks.find(key);
    if (handleIter != m_engineImpl->callbacks.end())
    {
        m_engineImpl->callbacks.at(key) = callback;
    }
    else
    {
        m_engineImpl->callbacks.emplace(std::make_pair(key, callback));
    }
}

void TCPCommunicationEngine::UnregisterConnection(const IChannel& connection)
{
    m_engineImpl->callbacks.erase(const_cast<IChannel*>(&connection));
}

engine_callback TCPCommunicationEngine::GetCallback(const IChannel& connection) const
{
    engine_callback callback;
    auto key = const_cast<IChannel*>(&connection);
    auto handleIter = m_engineImpl->callbacks.find(key);
    if (handleIter != m_engineImpl->callbacks.end())
    {
        callback = m_engineImpl->callbacks.at(key);
    }
    return callback;
}

void* TCPCommunicationEngine::GetInternal() const
{
    return &m_engineImpl->service;
}

std::shared_ptr<Network::ISocket> TCPCommunicationEngine::Create(bool)
{
    return std::make_shared<TCPSocket>(m_engineImpl->service);
}

void TCPCommunicationEngine::StartInternal()
{
    m_engineImpl->work = std::make_unique<boost::asio::io_context::work>(m_engineImpl->service);
    auto handler = [this] { m_engineImpl->service.run(); };
    GENGINE_POST_TASK(handler);
}

void TCPCommunicationEngine::StopInternal()
{
    m_engineImpl->service.stop();
    m_engineImpl->work.reset();
    Worker::Dispose();
}

}
}
