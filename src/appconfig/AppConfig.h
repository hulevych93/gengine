#pragma once

#include <api/services/ServiceType.h>
#include <api/appconfig/EntryConfig.h>
#include <api/appconfig/ProcessConfig.h>
#include <api/appconfig/PluginConfig.h>
#include <api/appconfig/ThreadConfig.h>
#include <api/appconfig/ServiceConfig.h>

namespace Gengine {
namespace AppConfig {

bool Merge(EntryConfig& left, const EntryConfig& right);

}
}