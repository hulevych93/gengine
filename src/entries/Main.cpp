#include <entries/Main.h>

#include <api/core/ILogger.h>
#include <api/core/IEnvironment.h>
#include <api/entries/IExecutor.h>

#include <entries/EntryRegistry.h>
#include <entries/ArgsVisitors.h>

#include <brokers/ProcessBroker.h>
#include <brokers/ServiceBroker.h>
#include <brokers/ExecutorBroker.h>
#include <brokers/PluginBroker.h>
#include <brokers/PersistencyBroker.h>
#include <brokers/WorkerBroker.h>

#include <shared-services/SharedServiceImport.h>

#include <appconfig/AppConfig.h>
#include <appconfig/BufferConfigReader.h>

#include <filesystem/Filesystem.h>
#include <core/Encoding.h>
#include <core/Logger.h>

#include <iostream>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#if defined(_WIN32)
    #include <Windows.h>
#endif

namespace bpo = boost::program_options;

namespace Gengine {
using namespace SharedServices;
using namespace Services;
using namespace AppConfig;

namespace Entries {

namespace {
std::unique_ptr<EntryConfig> GetOptionalConfig(args_type args);

class ConfigurationLock
{
public:
    ConfigurationLock(EntryConfig& config)
        : m_config(config)
        , m_configured(false)
    {}
    ~ConfigurationLock()
    {
        Deconfigure();
    }

    void Process(args_type args)
    {
        auto optionalConfig = GetOptionalConfig(args);
        if (optionalConfig)
        {
            Merge(m_config, *optionalConfig);
        }
        Configure();
    }

    void Configure()
    {
        if (!m_config.workers.empty())
        {
            GENGINE_INTIALIZE_CONCURRENCY(m_config.workers);
        }
        if (m_config.persistencyDir)
        {
            auto persistencyDir = Filesystem::CombinePath(toUtf8(Filesystem::GetAppFolder()), m_config.persistencyDir.get());
            INITIALIZE_PERSISTENCY(persistencyDir);
        }
        if (m_config.pluginsDir)
        {
            auto pluginsDir = Filesystem::CombinePath(Filesystem::GetAppFolder(), utf8toWchar(m_config.pluginsDir.get()));
            INITIALIZE_PLUGINS(pluginsDir);
        }
        if (!m_config.inServices.empty() || !m_config.outServices.empty())
        {
            INITIALIZE_SERVICES(m_config.inServices, m_config.outServices);
        }
        if (!m_config.outServices.empty())
        {
            INITIALIZE_EXECUTORS(m_config.outServices);
        }
        if (!m_config.processes.empty())
        {
            INITIALIZE_PROCESSES(m_config.processes);
        }
        if (m_config.consoleLogger)
        {
            if (m_config.consoleLogger.get())
                GLOG_INIT("", true);
            else
                GLOG_INIT(m_config.name.get());
        }
        m_configured = true;
    }
    void Deconfigure()
    {
        if (m_configured)
        {
            if (m_config.persistencyDir)
            {
                UNINITIALIZE_PERSISTENCY;
            }
            if (!m_config.processes.empty())
            {
                UNINITIALIZE_PROCESSES;
            }
            if (m_config.pluginsDir)
            {
                UNINITIALIZE_PLUGINS;
            }
            if (m_config.consoleLogger)
            {
                GLOG_DEINIT;
            }
            if (!m_config.inServices.empty())
            {
                UNINITIALIZE_SERVICES;
            }
            if (!m_config.outServices.empty())
            {
                UNINITIALIZE_EXECUTORS;
            }
            if (!m_config.workers.empty())
            {
                GENGINE_SHUTDOWN_CONCURRENCY;
            }
        }
    }

private:
    EntryConfig& m_config;
    bool m_configured;
};
}

const std::wstring Main::ProgramOptionsPattern(L"%1% options:");
Main* Main::Instance = nullptr;

Main::Main()
    : m_factory(makeFactory())
{
    assert(!Instance);
    Instance = this;

    IMRORT_SHARED_SERVICE(Logger);
    IMRORT_CREATOR_SHARED_SERVICE(Environment);
}

std::int32_t Main::Run(args_type args)
{
    auto executor = std::shared_ptr<IExecutor>();
    auto configLock = std::unique_ptr<ConfigurationLock>();

    m_entry = GetEntry(args);

    if (m_entry)
    {
        void* config_;
        m_entry->GetConfig(&config_);

        if (config_)
        {
            auto config = reinterpret_cast<EntryConfig*>(config_);
            configLock = std::make_unique<ConfigurationLock>(const_cast<EntryConfig&>(*config));
            configLock->Process(args);
        }

        std::vector<std::unique_ptr<ICmdProcessor>> processors;
        m_entry->CreateProcessors(&processors);
        for (auto& processor : processors)
        {
            RegisterProcessor(std::move(processor));
        }

        m_entry->CreateExecutor(&executor);

        if (executor)
        {
            std::vector<std::unique_ptr<ICmdProcessor>> processors;
            executor->CreateProcessors(&processors);
            for (auto& processor : processors)
            {
                RegisterProcessor(std::move(processor));
            }
        }
    }

    bool success = true;
    if (boost::apply_visitor(EmptyVisitor(), args) || !ProcessCmdLine(args, &success))
    {
        if (executor)
        {
            auto processes = std::vector<std::unique_ptr<ProcessHolder>>();

            void* config_;
            m_entry->GetConfig(&config_);
            auto config = reinterpret_cast<EntryConfig*>(config_);

            if (!config->processRefs.empty())
            {
                for (const auto& procRef : config->processRefs)
                {
                    auto holder = std::make_unique<ProcessHolder>(procRef);
                    holder->Run();
                    processes.push_back(std::move(holder));
                }
            }

            executor->Execute(&args);

            std::int32_t code;
            executor->GetCode(&code);
            success = code == 0;

            if (!config->processRefs.empty())
            {
                for (const auto& proc : processes)
                {
                    proc->Stop();
                }
            }
        }
    }

    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

void Main::RegisterProcessor(std::unique_ptr<ICmdProcessor>&& processor)
{
    std::wstring commandKey;
    processor->GetCommandKey(&commandKey);
    auto processorIter = m_optionsProcessors.find(commandKey);
    if (processorIter == m_optionsProcessors.end())
    {
        processor->SetEntry(m_entry.get());
        m_optionsProcessors.emplace(commandKey, std::move(processor));
    }
}

bool Main::ProcessCmdLine(args_type args, bool* result) const
{
    assert(result);
    *result = false;
    if (!m_optionsProcessors.empty())
    {
        std::wstring name;
        m_entry->GetAppName(&name);
        name = (boost::wformat(ProgramOptionsPattern) % name).str();

        for (const auto& optionIter : m_optionsProcessors)
        {
            auto command = optionIter.first;
            auto& processor = optionIter.second;

            bpo::options_description option_description(toUtf8(name));
            auto optionsObject = option_description.add_options();

            processor->Register(&optionsObject);

            bpo::variables_map vm;
            try
            {
                boost::apply_visitor(ParseVisitor(vm, option_description), args);
                bpo::notify(vm);
            }
            catch (const std::exception& ex)
            {
                std::cerr << "Failed to parse command-line: " << ex.what() << std::endl;
                return true;
            }

            if (vm.count(toUtf8(command)))
            {
                processor->Process(&args, result);
                return true;
            }
        }

        return false;
    }
    else
    {
        std::cout << "There is no options processors specified" << std::endl;
        return false;
    }
}

std::shared_ptr<IEntry> Main::GetEntry(args_type args) const
{
    EntryLock lock;
    std::wstring entryName;
    try
    {
        bpo::options_description option_description;
        auto optionsObject = option_description.add_options();
        bpo::variables_map vm;
        optionsObject("entry", bpo::wvalue<std::wstring>(&entryName), "entry");
        boost::apply_visitor(ParseVisitor(vm, option_description), args);
        bpo::notify(vm);

        GLOG_INFO_INTERNAL("Entry point parsed: %s", entryName);
    }
    catch (const std::exception& ex)
    {
        GLOG_ERROR_INTERNAL("Failed to parse command options for entry point: %s", ex.what());
    }

    return EntryRegistry::Create(std::move(m_factory), entryName);
}

namespace {
std::unique_ptr<EntryConfig> GetOptionalConfig(args_type args)
{
    std::string configBuffer;
    try
    {
        bpo::options_description option_description;
        auto optionsObject = option_description.add_options();
        bpo::variables_map vm;
        optionsObject("config", boost::program_options::value<std::string>(&configBuffer), "additional config");
        boost::apply_visitor(ParseVisitor(vm, option_description), args);
        bpo::notify(vm);

        if (!configBuffer.empty())
        {
            GLOG_INFO_INTERNAL("Additional config parsed: %s", configBuffer);

            auto config = std::make_unique<EntryConfig>();
            if (!BufferConfigReader(configBuffer, *config).Load())
                throw std::runtime_error("no service config loaded...");
            return config;
        }
    }
    catch (const std::exception& ex)
    {
        GLOG_ERROR_INTERNAL("Failed to parse command options for entry point: %s", ex.what());
        throw;
    }

    return std::unique_ptr<EntryConfig>();
}
}

}
}
