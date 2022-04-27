#include <api/core/ILogger.h>

#include <core/Logger.h>
#include <core/Encoding.h>

#include <entries/EntryRegistry.h>
#include <entries/SimpleExecutor.h>
#include <entries/Main.h>
#include <entries/EntryBase.h>

#include <brokers/ExecutorBroker.h>
#include <shared-services/SharedServiceExport.h>

#if defined(BUILD_WINDOWS)
#include <Windows.h>
#endif

using namespace Gengine;
using namespace Services;
using namespace Entries;

class LoggerHandler : public ILogger
{
public:
    bool Init(const std::wstring& alias, bool console) override
    {
        return true;
    }
    bool LogTrace(const std::wstring& message) override
    {
        return true;
    }
    bool LogError(const std::wstring& message) override
    {
        GLOG_ERROR(toUtf8(message));
        return true;
    }
    bool LogWarning(const std::wstring& message) override
    {
        GLOG_WARNING(toUtf8(message));
        return true;
    }
    bool LogInfo(const std::wstring& message) override
    {
        GLOG_INFO(toUtf8(message));
        return true;
    }
    bool LogDebug(const std::wstring& message) override
    {
        GLOG_DEBUG(toUtf8(message));
        return true;
    }
};

class ConsoleLoggerModule : public Gengine::Entries::SlaveEntry
{
public:
    ConsoleLoggerModule(std::unique_ptr<IEntryToolsFactory>&& factory) 
        : SlaveEntry(std::move(factory))
        , m_logger("CFC1399D")
    {}
    ~ConsoleLoggerModule() = default;

    bool Initialize() override 
    {
        std::wstring name;
        GetAppName(&name);
        //SetConsoleTitleA(toUtf8(name).c_str());
        return SlaveEntry::Initialize();
    }

    bool Execute(void* args) override 
    {
        m_logger.Reveal();
        SlaveEntry::Execute(args);
        return true;
    }

    bool Exit(std::int32_t* exitCode) override
    {
        assert(exitCode);
        GLOG_INFO("GLOGGER IS CLOSING...");
        std::this_thread::sleep_for(std::chrono::seconds(5));
        return SlaveEntry::Exit(exitCode);
    }

    bool Finalize() override
    {
        m_logger.Hide();
        SlaveEntry::Finalize();
        return true;
    }

    bool CreateExecutor(std::shared_ptr<IExecutor>* executor) override
    {
        assert(executor);
        *executor = std::make_unique<SimpleExecutor>(*this);
        return true;
    }

    bool CreateProcessors(std::vector<std::unique_ptr<ICmdProcessor>>*) override
    {
        return true;
    }

private:
    Gengine::Services::ServiceObjectProxy<LoggerHandler> m_logger;
};

REGISTER_MAIN_ENTRY(ConsoleLoggerModule)

IMPLEMENT_CONSOLE_ENTRY
