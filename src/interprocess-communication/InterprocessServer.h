#pragma once

#include <mutex>
#include <vector>
#include <string>
#include <thread>
#include <unordered_map>

#include <interprocess-communication/InterprocessCommonDefs.h>
#include <interprocess-communication/InterfaceExecutor.h>
#include <core/Runnable.h>

namespace Gengine {
namespace InterprocessCommunication {

class InputParameters;
class OutputParameters;
class IChannel;
class ChannelAgent;
class InterfaceImpl;
class InterprocessAcceptor;
class CommunicationEngine;

class InterprocessServer : public Runnable
{
public:
    InterprocessServer(const ipc_connection& connection, std::uint32_t threadId);
    virtual ~InterprocessServer();

    void Dispose();
    void AddExecutor(const std::shared_ptr<InterfaceImpl>& object);
    void RemoveExecutor(const std::shared_ptr<InterfaceImpl>& object);

protected:
    void StartInternal() override;
    void StopInternal() override;
    bool IsCanStart() override;
    void Accept();

protected:
    friend class ChannelAgent;
    void OnConnectionLost(const ChannelAgent* endpoint);
    ResponseCodes ProcessRequest(std::uint32_t function, const interface_key& binding,
        std::shared_ptr<const InputParameters> inputs,
        std::shared_ptr<OutputParameters> outputs) const;
    void ProcessEvent(std::uint32_t function, const interface_key& binding,
        std::shared_ptr<const InputParameters> inputs) const;

private:
    std::unique_ptr<InterprocessAcceptor> m_acceptor;
    std::shared_ptr<CommunicationEngine> m_engine;

    using TRPCClients = std::vector<std::unique_ptr<ChannelAgent>>;
    TRPCClients m_clients;

    mutable std::mutex m_mutex;
    using TExecuters = std::unordered_map<interface_key, std::shared_ptr<InterfaceExecutor>>;
    mutable TExecuters m_executers;
    using TListeners = std::unordered_map<interface_key, std::vector<std::shared_ptr<InterfaceListener>>>;
    mutable TListeners m_listeners;
};
}
}
