#pragma once

#include <interprocess-communication/pipes/Unix/UnixSocketEngine.h>

namespace Gengine {
namespace InterprocessCommunication {

class LinuxSocketEngine : public UnixSocketEngine {
 public:
  void PostRead(const IChannel& /*connection*/,
                void* /*data*/,
                std::uint32_t /*size*/) override {}
  void PostWrite(const IChannel& /*connection*/,
                 const void* /*data*/,
                 std::uint32_t /*size*/) override {}
};
}  // namespace InterprocessCommunication
}  // namespace Gengine
