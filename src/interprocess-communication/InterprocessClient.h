#pragma once

#include "InterprocessClientInterface.h"

namespace Gengine {
namespace InterprocessCommunication {
class IChannel;

/**
 * @brief The InterprocessClient class
 */
class InterprocessClient : public InterprocessClientInterface {
 public:
  InterprocessClient();
  ~InterprocessClient() override;

 public:
  /**
   * @brief Connect to the server
   * @param data to connect is either local or tcp/ip endpoint.
   * @return success or not
   */
  bool Connect(const ipc_connection& data) override;

  /**
   * @brief Dispose and disconnect from the server.
   */
  void Dispose() override;

 protected:
  bool SendRequest(const interface_key& interfaceKey,
                   std::uint8_t functionNumber,
                   InputParameters& results,
                   const OutputParameters& arguments) override;
  bool SendEvent(const interface_key& interfaceKey,
                 std::uint8_t functionNumber,
                 const OutputParameters& arguments) override;

 private:
  bool Send(const interface_key& interfaceKey,
            std::uint8_t functionNumber,
            bool request,
            const OutputParameters& arguments);

 private:
  std::unique_ptr<IChannel> m_impl;
};
}  // namespace InterprocessCommunication
}  // namespace Gengine
