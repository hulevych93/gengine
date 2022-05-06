#include <core/Logger.h>

#include <brokers/ServiceBroker.h>
#include <core/Encoding.h>
#include <shared-services/SharedServiceExport.h>

#include <api/core/ILogger.h>

namespace Gengine {
using namespace Services;

class LoggerProxy : public ILoggerProxy {
 public:
  void Init(const std::string& filename, bool useConsole) override {
    if (loggers.empty()) {
#if 0
            loggers.emplace(std::make_pair(type_t::Regular, std::make_unique<ServiceClientStub>("CFC1399D")));
#endif
      loggers.emplace(std::make_pair(
          type_t::Internal, std::make_unique<ServiceClientStub>("8EC6F366")));

      for (auto& stub : loggers)
        stub.second->Execute([filename, useConsole](const TService& service) {
          auto loggerService = std::dynamic_pointer_cast<ILogger>(service);
          loggerService->Init(utf8toWchar(filename), useConsole);
        });
    }
  }

  void Deinit() override { loggers.clear(); }

  void Log(type_t type, level_t level, const std::wstring& data) override {
    if (IsAvailable(type)) {
      auto& stub = loggers.at(type_t::Internal);
      switch (level) {
        case level_t::Trace:
          stub->Execute([data](const TService& service) {
            auto loggerService = std::dynamic_pointer_cast<ILogger>(service);
            loggerService->LogTrace(data);
          });
          break;
        case level_t::Error:
          stub->Execute([data](const TService& service) {
            auto loggerService = std::dynamic_pointer_cast<ILogger>(service);
            loggerService->LogError(data);
          });
          break;
        case level_t::Warning:
          stub->Execute([data](const TService& service) {
            auto loggerService = std::dynamic_pointer_cast<ILogger>(service);
            loggerService->LogWarning(data);
          });
          break;
        case level_t::Info:
          stub->Execute([data](const TService& service) {
            auto loggerService = std::dynamic_pointer_cast<ILogger>(service);
            loggerService->LogInfo(data);
          });
          break;
        case level_t::Debug:
          stub->Execute([data](const TService& service) {
            auto loggerService = std::dynamic_pointer_cast<ILogger>(service);
            loggerService->LogDebug(data);
          });
          break;
        default:
          break;
      }
    }
  }

  bool IsAvailable(type_t) const override {
    return loggers.find(type_t::Internal) != loggers.end();
  }

  std::map<type_t, std::unique_ptr<ServiceClientStub>> loggers;
};

EXPORT_GLOBAL_SHARED_SERVICE(LoggerProxy)

}  // namespace Gengine
