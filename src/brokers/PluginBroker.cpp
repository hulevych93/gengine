#include <brokers/PluginBroker.h>

#include <api/plugins/IPluginBroker.h>
#include <shared-services/SharedServiceImport.h>

namespace Gengine {
using namespace SharedServices;

namespace Services {

namespace {
std::shared_ptr<IPluginBroker> GetBroker() {
  static std::shared_ptr<IPluginBroker> PluginBroker;
  if (!PluginBroker) {
    SharedConnection data;
    data.path = "plugin-broker";
    data.symbol = "PluginBroker_service";
    PluginBroker = import_symbol<IPluginBroker>(data);
  }
  assert(PluginBroker);
  return PluginBroker;
}
}  // namespace

void InitializePlugins(const std::wstring& config) {
  GetBroker()->Load(config);
  GetBroker()->InitPlugins();
}

void DeinitializePlugins() {
  GetBroker()->DeinitPlugins();
  GetBroker()->Unload();
}

}  // namespace Services
}  // namespace Gengine
