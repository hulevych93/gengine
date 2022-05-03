#include <interprocess-communication/ServerInitializer.h>

#include <interprocess-communication/CommunicationEngine.h>
#include <interprocess-communication/InterprocessAcceptor.h>

#include <core/Logger.h>
#include <interprocess-communication/tcp/TCPAcceptor.h>
#include <interprocess-communication/tcp/TCPCommunicationEngine.h>

namespace Gengine {
namespace InterprocessCommunication {

ServerInitializer::ServerInitializer(
    std::shared_ptr<CommunicationEngine>& engine,
    std::unique_ptr<InterprocessAcceptor>& acceptor,
    std::uint32_t threadId)
    : m_threadId(threadId), m_engine(engine), m_acceptor(acceptor) {}

bool ServerInitializer::operator()(const PipeConnection& data) const {
  const std::wstring connectionString =
      std::wstring{ChannelAddressPrefix} + data.pipe;
  m_engine = makeEngine(m_threadId);
  m_acceptor = makeAcceptor(connectionString, m_engine);
  return true;
}

bool ServerInitializer::operator()(const TcpConnection& data) const {
  m_engine = std::make_shared<TCPCommunicationEngine>(m_threadId);
  m_acceptor = std::make_unique<TCPAcceptor>(data, m_engine);
  return true;
}

}  // namespace InterprocessCommunication
}  // namespace Gengine
