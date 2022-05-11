#pragma once

#include "InterprocessClientInterface.h"

namespace Gengine {
namespace InterprocessCommunication {

class InterprocessClient : public InterprocessClientInterface {
 public:
  InterprocessClient();
  virtual ~InterprocessClient();

 public:
  bool Connect(const ipc_connection& data) override;
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
