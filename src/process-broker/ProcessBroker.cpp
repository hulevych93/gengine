#include <brokers/ProcessBroker.h>

#include <unordered_map>
#include <boost/signals2.hpp>

#include <core/Encoding.h>
#include <appconfig/AppConfig.h>
#include <brokers/ServiceBroker.h>
#include <brokers/WorkerBroker.h>

#include <interprocess-syncronization/ExecutableLauncher.h>

#if _WIN32
#include <interprocess-syncronization/Windows/ServiceLauncher.h>
#endif

#include <shared-services/SharedServiceExport.h>

#include <api/core/ILifetime.h>
#include <api/services/IServiceRouter.h>
#include <core/Logger.h>

namespace Gengine {
using namespace InterprocessSynchronization;
using namespace Services;
using namespace AppConfig;

namespace Services {

using process_signal = boost::signals2::signal<void()>;

class ProcessLauncher : public Runnable
{
public:
    ProcessLauncher(IExecutableLauncherListener & listener, const std::shared_ptr<ExecutableLauncher>& launcher, const executable_params& params)
        : m_listener(listener)
        , m_launcher(launcher)
        , m_params(params)
    {}

protected:
    void StartInternal() override
    {
        m_launcher->AddExecutable(m_params, m_listener);
    }

    void StopInternal() override
    {
        m_launcher->RemoveExecutable(m_params);
    }

private:
    IExecutableLauncherListener & m_listener;
    std::shared_ptr<ExecutableLauncher> m_launcher;
    executable_params m_params;
};

class ProcessBroker: public IProcessBroker
    , public Worker
{
public:
    ProcessBroker()
        : Worker(11)
    {
        {
            ThreadConfig config;
            config.id = 11;
            config.name = "ProcessBrokerThread";
            GENGINE_REGISTER_THREAD(config);
        }
    }

    void Configure(const std::set<ProcessConfig>& config) override
    {
        auto task = [this, config]() {
            if (!m_launcher)
            {
                m_launcher = CreateExecutableLauncher();
                m_launcher->Start();
            }

            for (const auto& procConfig : config)
            {
                auto processContext = std::make_unique<ProcessContext>();
                processContext->id = procConfig.id;
                if (!procConfig.name.empty())
                {
                    executable_params params;
                    params.path = utf8toWchar(procConfig.name);
                    params.params = utf8toWchar(procConfig.params);
                    processContext->launcher = std::make_unique<ProcessLauncher>(*processContext, m_launcher, params);
                }
                else if (!procConfig.service.empty())
                {
#if _WIN32
                    processContext->launcher = std::make_unique<ServiceLauncher>(utf8toWchar(procConfig.service));
#endif
                }
                if (!procConfig.lifetimeRef.empty())
                {
                    processContext->lifetime = std::make_unique<ServiceClientProxy<ILifetime>>(procConfig.lifetimeRef);
                }
                signals.emplace(std::make_pair(procConfig.id, std::move(processContext)));
            }
        };
        POST_HEARTBEAT_WAITED_TASK(task);
    }

    void Deconfigure() override
    {
        auto task = [this]() {
            for (const auto& signal : signals)
            {
                TearDown(signal.first, true);
            }
            signals.clear();

            if (m_launcher)
            {
                m_launcher->Stop();
                m_launcher.reset();
            }
        };
        POST_HEARTBEAT_DEINITIALIZATION_TASK(task);
    }

    connection PowerUp(uint32_t id, IProcessClient& client) override
    {
        connection conns;

        auto task = [this, &conns, id, &client]() {
            auto iter = signals.find(id);
            if (iter != signals.end())
            {
                auto& context = iter->second;
                conns.emplace_back(context->launched.connect([&client] { client.OnProcessLauched(); }));
                conns.emplace_back(context->stopped.connect([&client] { client.OnProcessStopped(); }));
                context->launcher->Start();
                ++context->counter;
                if (context->running)
                {
                    client.OnProcessLauched();
                }
            }
        };
        POST_HEARTBEAT_WAITED_TASK(task);

        return conns;
    }

    void TearDown(uint32_t id, bool force) override
    {
        auto task = [this, id, force]() {
            auto iter = signals.find(id);
            if (iter != signals.end())
            {
                auto& context = iter->second;
                if (context->counter > 0)
                {
                    --context->counter;
                    if (context->counter == 0 || force)
                    {
                        if (context->lifetime)
                        {
                            auto success = false;
                            try
                            {
                                (*context->lifetime)->Shutdown(&success);
                            }
                            catch (std::exception& ex)
                            {
                                GLOG_WARNING_INTERNAL("(*context->lifetime)->Shutdown(&success) call failed: %s", ex.what());
                            }
                        }
                        context->launcher->Stop();
                    }
                }
            }
        };
        if (force)
        {
            POST_HEARTBEAT_WAITED_TASK(task);
        }
        else
        {
            POST_HEARTBEAT_TASK(task);
        }
    }

private:
    struct ProcessContext : IExecutableLauncherListener
    {
        void OnExecutableLaunched(const std::shared_ptr<Executable>& app) override
        {
            running = true;
            launched();
        }

        void OnExecutableClosed(const std::shared_ptr<Executable>& app) override
        {
            running = false;
            stopped();
        }

        bool running = false;
        std::uint32_t id = 0;
        std::uint32_t counter = 0;
        process_signal launched;
        process_signal stopped;
        std::unique_ptr<Runnable> launcher;
        std::unique_ptr<ServiceClientProxy<ILifetime>> lifetime;
    };
    std::map<std::uint32_t, std::unique_ptr<ProcessContext>> signals;
    std::shared_ptr<ExecutableLauncher> m_launcher;
};

EXPORT_GLOBAL_SHARED_SERVICE(ProcessBroker)

}
}
