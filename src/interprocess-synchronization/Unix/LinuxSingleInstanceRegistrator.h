#pragma once

#include <interprocess-synchronization/InstanceRegistratorInterface.h>

namespace Gengine {
namespace InterprocessSynchronization {
class LinuxSingleInstanceRegistrator : public InstanceRegistratorInterface {
 public:
  LinuxSingleInstanceRegistrator(std::wstring&& objectName);
  LinuxSingleInstanceRegistrator(const LinuxSingleInstanceRegistrator&) =
      delete;
  LinuxSingleInstanceRegistrator(LinuxSingleInstanceRegistrator&&) = delete;
  virtual ~LinuxSingleInstanceRegistrator();

  bool RegisterInstance() override;
  void UnregisterInstance() override;
  bool IsInstanceRegistered() const override;

 private:
  int m_file;
};
}  // namespace InterprocessSynchronization
}  // namespace Gengine
