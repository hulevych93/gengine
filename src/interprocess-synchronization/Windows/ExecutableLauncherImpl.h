#pragma once

#include <api/core/IServiceControlListener.h>
#include <brokers/ExecutorBroker.h>
#include <interprocess-synchronization/ExecutableLauncher.h>

namespace Gengine {

namespace InterprocessSynchronization {
class ExecutableLauncherImpl : public ExecutableLauncher,
                               public IServiceControlListener {
 public:
  ExecutableLauncherImpl();

  ExecutableLauncherImpl(const ExecutableLauncherImpl&) = delete;
  ExecutableLauncherImpl(ExecutableLauncherImpl&&) = delete;

  virtual ~ExecutableLauncherImpl();

  void AddExecutable(const executable_params& params,
                     IExecutableLauncherListener& listener) override;
  void RemoveExecutable(const executable_params& params) override;

 protected:
  bool OnHardwareProfileChange() override;
  bool OnPowerEvent() override;
  bool OnSessionChange(std::uint32_t event, std::uint32_t sessionId) override;

 protected:
  void OnExecutableLaunched(const std::shared_ptr<Executable>& app) override;
  void OnExecutableClosed(const std::shared_ptr<Executable>& app) override;

 protected:
  void StartInternal() override;
  void StopInternal() override;
  void Loop();

 private:
  Services::ServiceObjectProxy<IServiceControlListener&> m_serviceObj;

  void* m_stopEvent;
  void* m_changeEvent;

  using TWaitEvents = std::vector<void*>;
  TWaitEvents m_eventPool;

  std::uint32_t m_loopId;
};
}  // namespace InterprocessSynchronization
}  // namespace Gengine