#include <entries/PluginBase.h>

#include <api/plugins/PluginInfo.h>

#include <appconfig/AppConfig.h>
#include <appconfig/SelfExtractedBufferedConfigReader.h>

#include <core/Encoding.h>
#include <core/Logger.h>
#include <filesystem/Filesystem.h>

extern void* g_module_instance;

namespace Gengine {
using namespace AppConfig;

PluginBase::PluginBase() = default;

bool PluginBase::GetConfig(void** config) {
  assert(config);

  if (!m_config) {
    m_config = std::make_unique<PluginConfig>();
    if (!SelfExtractedBufferedConfigReader(*m_config).Load())
      throw std::runtime_error("no service config loaded...");
  }

  *config = m_config.get();

  return true;
}

bool PluginBase::Handshake(PluginInfo* info) {
  assert(info);

  void* config_;
  GetConfig(&config_);
  auto config = reinterpret_cast<PluginConfig*>(config_);

  if (config) {
    auto config = reinterpret_cast<PluginConfig*>(config_);
    if (config->name)
      info->name = utf8toWchar(config->name.get());
    if (config->description)
      info->description = utf8toWchar(config->description.get());

    info->path = toUtf8(Filesystem::GetFileName(
        Filesystem::GetModuleFilePath(g_module_instance)));
  }

  return true;
}

}  // namespace Gengine
