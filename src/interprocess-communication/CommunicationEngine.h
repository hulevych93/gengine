#pragma once

#include <interprocess-communication/InterprocessCommonDefs.h>

#include <core/Runnable.h>

namespace Gengine {
namespace InterprocessCommunication {
class IChannel;

class CommunicationEngine : public Runnable {
 public:
  virtual ~CommunicationEngine() = default;
  virtual void RegisterConnection(const IChannel& connection,
                                  engine_callback callback) = 0;
  virtual void UnregisterConnection(const IChannel& connection) = 0;
};

std::unique_ptr<CommunicationEngine> makeEngine(std::uint32_t threadId);

}  // namespace InterprocessCommunication
}  // namespace Gengine
