#pragma once

namespace Gengine {
namespace InterprocessSynchronization {
class IAliveObject {
 public:
  virtual ~IAliveObject() = default;
  virtual void Free() = 0;
  virtual bool IsLocked() const = 0;
};
}  // namespace InterprocessSynchronization
}  // namespace Gengine