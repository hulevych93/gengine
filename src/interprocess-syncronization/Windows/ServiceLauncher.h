#pragma once

#include <brokers/WorkerBroker.h>
#include <core/Runnable.h>

namespace Gengine {
namespace InterprocessSynchronization {
class ServiceLauncher : public Runnable, public Services::Worker {
 public:
  explicit ServiceLauncher(const std::wstring& serviceName);
  virtual ~ServiceLauncher() = default;

 protected:
  void CheckServicesRoutine();

 protected:
  void StartInternal() override;
  void StopInternal() override;
  bool IsCanStart() override;

 private:
  const std::wstring m_serviceName;

 private:
  std::uint32_t m_checkAppsTimerId;
  static const std::uint32_t CheckTimeout;
};
}  // namespace InterprocessSynchronization
}  // namespace Gengine
