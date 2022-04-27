#pragma once

#include <api/plugins/IPlugin.h>

namespace Gengine {

struct PluginConfig;

class PluginBase : public IPlugin
{
public:
    PluginBase();

    bool GetConfig(void** config) override;
    bool Handshake(PluginInfo* info) override;

private:
    mutable std::unique_ptr<PluginConfig> m_config;
};

}