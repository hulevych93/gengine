#pragma once

#include <memory>

namespace Gengine {
namespace InterprocessSynchronization {
class Executable;
class IExecutableLauncherListener {
 public:
  virtual void OnExecutableLaunched(const std::shared_ptr<Executable>& app) {}
  virtual void OnExecutableClosed(const std::shared_ptr<Executable>& app) {}
};
}  // namespace InterprocessSynchronization
}  // namespace Gengine