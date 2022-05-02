#pragma once

#include <string>

namespace Gengine {
namespace Services {

void InitializePlugins(const std::wstring& config);
void DeinitializePlugins();

#define GENGINE_INITIALIZE_PLUGINS(config) InitializePlugins(config)
#define GENGINE_UNGENGINE_INITIALIZE_PLUGINS DeinitializePlugins()

}
}
