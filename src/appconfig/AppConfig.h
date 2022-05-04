#pragma once

#include <api/appconfig/EntryConfig.h>
#include <api/appconfig/PluginConfig.h>
#include <api/appconfig/ProcessConfig.h>
#include <api/appconfig/ServiceConfig.h>
#include <api/appconfig/ThreadConfig.h>
#include <api/services/ServiceType.h>

namespace Gengine {
namespace AppConfig {

bool Merge(EntryConfig& left, const EntryConfig& right) noexcept;

}
}  // namespace Gengine
