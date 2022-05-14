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

/**
 * @brief The InterprocessServer class
 *
 * The server class implementation.
 */
class InterprocessServer final : public Runnable {
 public:
  /**
   * @brief InterprocessServer
   * @param connection of the server's acceptor.
   * @param threadId is the workrt thread in which the server is run.
   */
  InterprocessServer(const ipc_connection& connection, std::uint32_t threadId);

  /**
   * @brief ~InterprocessServer
   */
  virtual ~InterprocessServer();

  /**
   * @brief Dispose the server
   */
  void Dispose();

  /**
   * @brief AddExecutor
   * @param object of the server listener or executor.
   */
  void AddExecutor(const std::shared_ptr<InterfaceImpl>& object);

  /**
   * @brief RemoveExecutor
   * @param object of the server listener or executor.
   */
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
