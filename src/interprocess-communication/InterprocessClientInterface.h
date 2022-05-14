#pragma once

#include <interprocess-communication/InterprocessCommonDefs.h>

#include <boost/optional.hpp>

namespace Gengine {
namespace InterprocessCommunication {
class InputParameters;
class OutputParameters;

/**
 * @brief The InterprocessClientInterface class
 */
class InterprocessClientInterface {
 public:
  virtual ~InterprocessClientInterface() = default;

  /**
   * @brief Connect to the server
   * @param data to connect is either local or tcp/ip endpoint.
   * @return success or not
   */
  virtual bool Connect(const ipc_connection& data) = 0;

  /**
   * @brief Dispose and disconnect from the server.
   */
  virtual void Dispose() = 0;

 protected:
  virtual bool SendRequest(const interface_key& interfaceKey,
                           std::uint8_t functionNumber,
                           InputParameters& results,
                           const OutputParameters& arguments) = 0;
  virtual bool SendEvent(const interface_key& interfaceKey,
                         std::uint8_t functionNumber,
                         const OutputParameters& arguments) = 0;
};
}  // namespace InterprocessCommunication
}  // namespace Gengine
