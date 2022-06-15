#pragma once

#include <interprocess-synchronization/IAliveObject.h>

#include <string>

namespace Gengine {
namespace InterprocessSynchronization {
class DaemonAliveObject : public IAliveObject {
 public:
  explicit DaemonAliveObject(const std::wstring& mappingFileName);
  ~DaemonAliveObject();

  void Free() override;
  bool IsLocked() const override;

 private:
  int m_aliveFile;
};
}  // namespace InterprocessSynchronization
}  // namespace Gengine
