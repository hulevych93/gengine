#pragma once

#include <api/core/ISessionQuery.h>
#include <brokers/ServiceBroker.h>
#include <brokers/WorkerBroker.h>
#include <core/Runnable.h>
#include <interprocess-synchronization/Executable.h>
#include <interprocess-synchronization/IExecutableLauncherListener.h>
#include <unordered_map>

namespace Gengine {
namespace InterprocessSynchronization {
class ExecutableLauncher : public Runnable,
                           public Services::Worker,
                           public IExecutableLauncherListener {
 public:
  ExecutableLauncher();
  virtual ~ExecutableLauncher() = default;

  virtual void AddExecutable(const executable_params& params,
                             IExecutableLauncherListener& listener);
  virtual void RemoveExecutable(const executable_params& params);

 private:
  void WaitForStop(const executable_params& params);
  virtual std::vector<SessionId> GetActiveSessionKeys() const;

 protected:
  void StartInternal() override;
  void StopInternal() override;
  bool IsCanStart() override;
  void CheckAppsRoutine();
  void CheckDeadApps();
  void RunApps();

  void OnExecutableLaunched(const std::shared_ptr<Executable>& app) override;
  void OnExecutableClosed(const std::shared_ptr<Executable>& app) override;

 protected:
  using TExecutableInstancesMap =
      std::unordered_map<std::pair<SessionId, executable_params>,
                         std::shared_ptr<Executable>>;
  using TExecutablesMap =
      std::unordered_map<executable_params, IExecutableLauncherListener&>;

  TExecutableInstancesMap m_executableInstances;
  TExecutablesMap m_executablesMap;

  Services::ServiceClientProxy<ISessionQuery> m_sessionQuery;

 private:
  std::uint32_t m_checkAppsTimerId;
  static const std::uint32_t CheckTimeout;
  static const std::uint32_t TryCounterMax;
};

std::shared_ptr<ExecutableLauncher> CreateExecutableLauncher();
}  // namespace InterprocessSynchronization
}  // namespace Gengine
