
#include <memory>

#include <brokers/ExecutorBroker.h>
#include <brokers/ServiceBroker.h>
#include <brokers/WorkerBroker.h>

#include <entries/Main.h>

#include <core/Logger.h>

#if defined(_WIN32)
#include <Windows.h>
#endif

using namespace Gengine;
using namespace Gengine::Services;
using namespace Gengine::Entries;

class TestService final : public Entries::EntryBase,
                 public Worker
{
public:
    TestService(std::unique_ptr<Entries::IEntryToolsFactory>&& factory)
        : EntryBase(std::move(factory))
        , Worker(0)
    {

    }

    ~TestService() = default;

    bool Initialize() override
    {
        return true;
    }

    bool Execute(void* args) override
    {
        assert(args);

        auto handler = [&]()
        {
            GLOG_INFO("I'm alive");
        };

        START_HEARTBEAT_TIMER(std::move(handler), 5000);

        return true;
    }

    bool Exit(std::int32_t* exitCode) override
    {
        assert(exitCode);

        STOP_HEARTBEAT_TIMER_WITH_WAIT(m_timerId);

        *exitCode = 0;

        return true;
    }

    bool Finalize() override
    {
        return true;
    }

    bool CreateExecutor(std::shared_ptr<IExecutor>* executor) override
    {
        assert(executor);
        *executor = makeServiceExecutor(*this);
        return true;
    }

    bool CreateProcessors(std::vector<std::unique_ptr<Gengine::ICmdProcessor>>* processors) override
    {
        assert(processors);
        return true;
    }

private:
    std::int32_t m_timerId;
};

REGISTER_MAIN_ENTRY(TestService)
IMPLEMENT_CONSOLE_ENTRY
