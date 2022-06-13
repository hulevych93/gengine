#pragma once

#include <api/core/IServiceControlListener.h>
#include <brokers/ServiceBroker.h>
#include <entries/CmdProcessorBase.h>
#include <entries/SimpleExecutor.h>

struct _SERVICE_STATUS;
typedef _SERVICE_STATUS SERVICE_STATUS;
struct tagWTSSESSION_NOTIFICATION;
typedef tagWTSSESSION_NOTIFICATION WTSSESSION_NOTIFICATION;

namespace Gengine {
namespace Entries {
class ServiceExecutor : public SimpleExecutor {
 public:
  ServiceExecutor(const ServiceExecutor&) = delete;
  ServiceExecutor(IEntry& entry);
  ~ServiceExecutor();

  bool Execute(void* args) override;
  bool CreateProcessors(
      std::vector<std::unique_ptr<ICmdProcessor>>* processors) override;

 protected:
  virtual void OnStop();
  virtual void OnPause();
  virtual void OnContinue();
  virtual void OnInterrogate();
  virtual void OnShutdown();
  virtual void OnHardwareProfileChange();
  virtual void OnPowerEvent();
  virtual void OnSessionChange(std::uint32_t reason,
                               WTSSESSION_NOTIFICATION* notification);

 private:
  void ServiceMain(std::uint32_t argc, wchar_t** argv);
  std::uint32_t HandlerEx(std::uint32_t control,
                          std::uint32_t eventType,
                          void* eventData,
                          void* context);

 private:
  static void __stdcall _ServiceMain(unsigned long dwArgc, wchar_t* lpszArgv[]);
  static unsigned long __stdcall _HandlerEx(unsigned long dwControl,
                                            unsigned long dwEventType,
                                            void* lpEventData,
                                            void* lpContext);

 private:
  struct StatusContext {
    StatusContext();
    ~StatusContext() = default;
    void Update(std::uint32_t code);
    void* handle;
    std::unique_ptr<SERVICE_STATUS> status;
  };

 protected:
  class SWindowsServiceInstallCommand : public CmdProcessorBase {
   public:
    SWindowsServiceInstallCommand();
    bool Process(void* args, bool* success) override;
  };

  class SWindowsServiceUninstallCommand : public CmdProcessorBase {
   public:
    SWindowsServiceUninstallCommand();
    bool Process(void* args, bool* success) override;
  };

  class SWindowsServiceStartCommand : public CmdProcessorBase {
   public:
    SWindowsServiceStartCommand();
    bool Process(void* args, bool* success) override;
  };

  class SWindowsServiceStopCommand : public CmdProcessorBase {
   public:
    SWindowsServiceStopCommand();
    bool Process(void* args, bool* success) override;
  };

  class SWindowsServiceDebugCommand : public CmdProcessorBase {
   public:
    SWindowsServiceDebugCommand();
    bool Process(void* args, bool* success) override;
  };

 private:
  StatusContext m_status;
  void* m_stopEvent;
  Services::ServiceClientProxy<IServiceControlListener> m_controlListener;

 private:
  static ServiceExecutor* m_instance;
};
}  // namespace Entries
}  // namespace Gengine