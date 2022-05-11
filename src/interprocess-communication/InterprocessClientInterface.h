#pragma once

#include <interprocess-communication/InterprocessCommonDefs.h>

#include <boost/optional.hpp>

namespace Gengine {
namespace InterprocessCommunication {
class InputParameters;
class OutputParameters;

class InterprocessClientInterface {
 public:
  virtual ~InterprocessClientInterface() = default;
  virtual bool Connect(const ipc_connection& data) = 0;
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
