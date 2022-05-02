#include "InterprocessServer.h"

#include "InterprocessAcceptor.h"
#include "InputParameters.h"
#include "OutputParameters.h"
#include "ChannelAgent.h"
#include "InterfaceExecutor.h"
#include "CommunicationEngine.h"
#include "ServerInitializer.h"

#include <core/Encoding.h>
#include <core/Logger.h>

namespace Gengine {
namespace InterprocessCommunication {

InterprocessServer::InterprocessServer(const ipc_connection& connection, std::uint32_t threadId)
{
    boost::apply_visitor(ServerInitializer(m_engine, m_acceptor, threadId), connection);
}

InterprocessServer::~InterprocessServer() = default;

void InterprocessServer::AddExecutor(const std::shared_ptr<InterfaceImpl>& object)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (auto executer = std::dynamic_pointer_cast<InterfaceExecutor>(object))
    {
        m_executers.emplace(std::make_pair(executer->GetInterface(), executer));
    }
    else if (auto listener = std::dynamic_pointer_cast<InterfaceListener>(object))
    {
        auto listenersIter = m_listeners.find(listener->GetInterface());
        if (listenersIter == m_listeners.end())
        {
            m_listeners.emplace(std::make_pair(listener->GetInterface(), std::vector<std::shared_ptr<InterfaceListener>>()));
        }
        m_listeners.at(listener->GetInterface()).emplace_back(listener);
    }
    else
    {
        GLOG_ERROR_INTERNAL("Failed to register InterfaceImpl");
    }
}

void InterprocessServer::RemoveExecutor(const std::shared_ptr<InterfaceImpl>& object)
{
    std::lock_guard<std::mutex> lock(m_mutex);
}

bool InterprocessServer::IsCanStart()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return !m_executers.empty() || !m_listeners.empty();
}

void InterprocessServer::Dispose()
{
    m_executers.clear();
    m_listeners.clear();
}

void InterprocessServer::StartInternal()
{
    Accept();
    m_engine->Start();
}

void InterprocessServer::StopInternal()
{
    m_engine->Stop();
    m_clients.clear();
}

void InterprocessServer::Accept()
{
    m_acceptor->AcceptConnection([this](bool success, std::unique_ptr<IChannel>&& channel) {
        if (success)
        {
            auto agent = std::make_unique<ChannelAgent>(std::move(channel), *this);

            auto handler = [this, &agent = *agent, queued = true]
                (bool success, std::uint32_t bytesProcessed, const bool needToQueue) mutable
            {
                if (!success)
                {
                    OnConnectionLost(&agent);
                    return;
                }

                if(needToQueue)
                {
                    success = agent.HandleIO(bytesProcessed);
                    if (!success)
                    {
                        OnConnectionLost(&agent);
                        return;
                    }

                    success = queued = agent.QueueIO();
                }
                else
                {
                    if (queued)
                    {
                        queued = false;
                        success = agent.HandleIO(bytesProcessed);
                    }
                    else
                    {
                        success = queued = agent.QueueIO();
                    }
                }

                if(!success)
                {
                    OnConnectionLost(&agent);
                    return;
                }
            };

            m_engine->RegisterConnection(agent->GetChannel(),
                                         std::move(handler));

            if (agent->QueueIO())
            {
                m_clients.emplace_back(std::move(agent));
            }
            else
            {
                m_engine->UnregisterConnection(agent->GetChannel());
            }

            Accept();
        }
    });
}

void InterprocessServer::OnConnectionLost(const ChannelAgent* endpoint)
{
   auto iter = std::find_if(m_clients.begin(), m_clients.end(), [endpoint](const TRPCClients::value_type& client) {
      return client.get() == endpoint;
   });
   if (iter != m_clients.end())
   {
      const auto& agent = *iter;
      m_engine->UnregisterConnection(agent->GetChannel());
      m_clients.erase(iter);
   }
}

ResponseCodes InterprocessServer::ProcessRequest(std::uint32_t function, const interface_key& binding,
    std::shared_ptr<const InputParameters> inputs,
    std::shared_ptr<OutputParameters> outputs) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto executorIter = m_executers.find(binding);
    if (executorIter != m_executers.end())
    {
        return executorIter->second->HandleRequest(function, inputs, outputs);
    }
    return ResponseCodes::UnknownInterface;
}

void InterprocessServer::ProcessEvent(std::uint32_t function, const interface_key& binding, std::shared_ptr<const InputParameters> inputs) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto executorIter = m_executers.find(binding);
    if (executorIter != m_executers.end())
    {
        executorIter->second->HandleEvent(function, inputs);
    }
    auto listenersIter = m_listeners.find(binding);
    if (listenersIter != m_listeners.end())
    {
        for (auto& listener : listenersIter->second)
        {
            listener->HandleEvent(function, inputs);
        }
    }
}

}
}
