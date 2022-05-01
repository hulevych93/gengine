#pragma once

#include <multithreading/Event.h>
#include <brokers/ExecutorBroker.h>
#include <interprocess-syncronization/ExecutableLauncher.h>
#include <api/core/IServiceControlListener.h>

namespace Gengine {

namespace InterprocessSynchronization {
class ExecutableLauncherImpl : public ExecutableLauncher,
   public IServiceControlListener
{
public:
   ExecutableLauncherImpl();
   virtual ~ExecutableLauncherImpl() = default;

   void AddExecutable(const executable_params& params, IExecutableLauncherListener& listener) override;
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

   Multithreading::Event m_StopEvent;
   Multithreading::Event m_ChangeEvent;

   using TWaitEvents = std::vector<void*>;
   TWaitEvents m_eventPool;

   std::uint32_t m_loopId;
};
}
}