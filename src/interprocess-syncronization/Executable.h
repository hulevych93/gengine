#pragma once

#include <api/core/SessionId.h>
#include <interprocess-syncronization/InterprocessSynchronizationCommon.h>
#include <cstdint>

namespace Gengine {
namespace InterprocessSynchronization {

class Executable {
 public:
  explicit Executable(const executable_params& params);

  void Launch(SessionId key);
  std::uint32_t GetPid() const;
  void* GetHandle() const;
  bool IsAlive();
  void Kill();

  SessionId GetSessionKey() const;
  const executable_params& GetParams() const;

 protected:
  const executable_params m_params;
  void* m_handle;
  std::uint32_t m_pid;
  SessionId m_key;
};
}  // namespace InterprocessSynchronization
}  // namespace Gengine