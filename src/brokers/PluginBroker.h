#pragma once

#include <string>

namespace Gengine {
namespace Services {

void InitializePlugins(const std::wstring& config);
void DeinitializePlugins();

#define INITIALIZE_PLUGINS(config) InitializePlugins(config)
#define UNINITIALIZE_PLUGINS DeinitializePlugins()

}
}