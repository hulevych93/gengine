#include <api/plugins/IPluginBroker.h>
#include <api/plugins/PluginInfo.h>
#include <api/plugins/IPlugin.h>

#include <core/Logger.h>
#include <core/Encoding.h>

#include <appconfig/AppConfig.h>
#include <brokers/ServiceBroker.h>
#include <brokers/ExecutorBroker.h>
#include <brokers/PersistencyBroker.h>
#include <brokers/WorkerBroker.h>

#include <shared-services/SharedServiceExport.h>
#include <shared-services/SharedServiceImport.h>

#include <filesystem/FileSearcher.h>

#if defined(_WIN32)
    #include <Windows.h>
#endif

using namespace Gengine;
using namespace AppConfig;
using namespace Services;
using namespace SharedServices;
using namespace Serialization;

class PluginBroker : public IPluginBroker,
                     public Persistable<ISerializable>
{
public:
    PluginBroker()
        : Persistable("PluginBroker")
    {}

    bool Load(const std::wstring& path) override
    {
        Persistable::Load();

        if (m_plugins.empty())
        {
            auto pluginFiles = FileSeacher::Search(path, L".*\\.dll");
            for (const auto& pluginFile : pluginFiles)
            {
                onPlugin(toUtf8(pluginFile->FileName()));
            }
        }
        else
        {
            for (const auto& pluginIter : m_plugins)
            {
                onPlugin(pluginIter.first.path);
            }
        }

        return true;
    }

    bool Unload() override
    {
        for (const auto& pluginIter : m_plugins)
        {
            pluginIter.second->Finalize();
        }
        m_plugins.clear();
        return true;
    }

    bool InitPlugins() override
    {
        for (const auto& pluginIter : m_plugins)
        {
            pluginIter.second->Initialize();
        }
        return true;
    }

    bool DeinitPlugins() override
    {
        for (const auto& pluginIter : m_plugins)
        {
            pluginIter.second->Finalize();
        }
        return true;
    }

    bool GetCount(std::uint32_t* count) override
    {
        assert(count);
        *count = m_plugins.size();
        return true;
    }

protected:
    void onPlugin(const std::string& path)
    {
        try
        {
            SharedConnection data;
            data.path = path;
            data.symbol ="Plugin_service";
            auto plugin = import_symbol<IPlugin>(data);

            PluginInfo info;
            if (plugin->Handshake(&info))
            {
                void* config_;
                plugin->GetConfig(&config_);
                if (config_)
                {
                    auto config = reinterpret_cast<PluginConfig*>(config_);
                    if (!config->inServices.empty() || !config->outServices.empty())
                    {
                        INITIALIZE_SERVICES(config->inServices, config->outServices);
                    }
                    if (!config->outServices.empty())
                    {
                        INITIALIZE_EXECUTORS(config->outServices);
                    }
                    if (!config->workers.empty())
                    {
                        GENGINE_INTIALIZE_CONCURRENCY(config->workers);
                    }
                }

                m_plugins[info] = plugin;
            }
            else
            {
                GLOG_WARNING("Failed to handshake plugin: %ls", path);
            }
        }
        catch (std::exception& ex)
        {
            GLOG_WARNING("Failed to load plugin: %ls, what: %s", path, ex.what());
        }
    }

protected:
    bool Serialize(Serializer& serializer) const override
    {
        serializer << m_plugins.size();
        for (const auto& pluginIter : m_plugins)
        {
            serializer << pluginIter.first;
        }
        return true;
    }

    bool Deserialize(const Deserializer& deserializer) override
    {
        std::size_t size;
        PluginInfo info;
        deserializer >> size;
        while (size > 0)
        {
            deserializer >> info;
            m_plugins.emplace(info, nullptr);
            --size;
        }
        return true;
    }

private:
    std::unordered_map<PluginInfo, TIPlugin> m_plugins;
};

EXPORT_GLOBAL_SHARED_SERVICE(PluginBroker)
