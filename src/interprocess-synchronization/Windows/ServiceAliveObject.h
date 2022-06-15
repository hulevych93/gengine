#pragma once

#include <interprocess-synchronization/IAliveObject.h>

namespace Gengine {
namespace InterprocessSynchronization {
class ServiceAliveObject : public IAliveObject {
 public:
  explicit ServiceAliveObject(const wchar_t* mappingFileName);
  ~ServiceAliveObject();

  void Free() override;
  bool IsLocked() const override;

 private:
  void* m_serviceFileMappingHandle;
  void* m_serviceFileMappingData;
};
}  // namespace InterprocessSynchronization
}  // namespace Gengine