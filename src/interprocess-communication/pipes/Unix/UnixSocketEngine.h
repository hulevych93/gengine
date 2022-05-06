#pragma once

#include <interprocess-communication/CommunicationEngine.h>

namespace Gengine {
namespace InterprocessCommunication {

class UnixSocketEngine : public CommunicationEngine {
 public:
  virtual ~UnixSocketEngine() = default;

  virtual void PostRead(const IChannel& connection,
                        void* data,
                        std::uint32_t size) = 0;
  virtual void PostWrite(const IChannel& connection,
                         const void* data,
                         std::uint32_t size) = 0;
};
}  // namespace InterprocessCommunication
}  // namespace Gengine
