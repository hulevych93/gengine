#pragma once

#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <core/Runnable.h>
#include <interprocess-communication/InterfaceExecutor.h>
#include <interprocess-communication/InterprocessCommonDefs.h>

namespace Gengine {
namespace InterprocessCommunication {

class InputParameters;
class OutputParameters;
class IChannel;
class ChannelAgent;
class InterfaceImpl;
class InterprocessAcceptor;
class CommunicationEngine;

class InterprocessServer : public Runnable {
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
  ResponseCodes ProcessRequest(std::uint32_t function,
                               const interface_key& binding,
                               const InputParameters& inputs,
                               OutputParameters& outputs) const;
  void ProcessEvent(std::uint32_t function,
                    const interface_key& binding,
                    const InputParameters& inputs) const;

 private:
  std::unique_ptr<InterprocessAcceptor> m_acceptor;
  std::shared_ptr<CommunicationEngine> m_engine;

  using Clients = std::vector<std::unique_ptr<ChannelAgent>>;
  Clients m_clients;

  mutable std::mutex m_mutex;
  using ExecutersType =
      std::unordered_map<interface_key, std::shared_ptr<InterfaceExecutor>>;
  mutable ExecutersType m_executers;

  using ListenersType =
      std::unordered_map<interface_key,
                         std::vector<std::shared_ptr<InterfaceListener>>>;
  mutable ListenersType m_listeners;
};
}  // namespace InterprocessCommunication
}  // namespace Gengine
