
#include <memory>
#include <entries/EntryBase.h>
#include <brokers/ExecutorBroker.h>
#include <brokers/ServiceBroker.h>
#include <brokers/WorkerBroker.h>

#include <entries/EntryRegistry.h>
#include <entries/Main.h>
#include <entries/SimpleExecutor.h>

#include <core/Logger.h>

#include <api/echo/IEcho.h>

#if defined(_WIN32)
#include <Windows.h>
#endif

using namespace Gengine;
using namespace Gengine::Services;
using namespace Gengine::Entries;

class EchoServer : public Echo::IEcho,
                   Worker
{
public:
    EchoServer(const char* client, const char* server)
        : Worker(200)
        , m_echoClient(client)
        , m_echoServer(server, *this)
    {

    }

    void init()
    {
        m_echoServer.Reveal();
    }

    void deinit()
    {
        m_echoServer.Hide();
    }

public:
    bool echo(const std::wstring& hello) override
    {
        auto handler = [=]()
        {
            m_echoClient->echo(hello);
            GLOG_INFO("received");
        };
        GENGINE_POST_TASK(handler);
        return true;
    }

private:
    ServiceObjectProxy<Echo::IEcho&> m_echoServer;
    ServiceClientProxy<Echo::IEcho> m_echoClient;
};

class TestEcho : public Entries::EntryBase,
                 public Echo::IEcho
{
public:
    TestEcho(std::unique_ptr<Entries::IEntryToolsFactory>&& factory)
        : EntryBase(std::move(factory))
#if 1
        , m_echoClient("83F52161")
        , m_echoServer("3E0474C1", *this)
        , m_echo("3E0474C1", "83F52161")
#else
        , m_echoClient("A2H11HJK")
        , m_echoServer("SF345RHJ", *this)
        , m_echo("SF345RHJ", "A2H11HJK")
#endif
    {

    }

    ~TestEcho() = default;

    bool Initialize() override
    {
        m_echoServer.Reveal();
        m_echo.init();
        return true;
    }

    bool Execute(void* args) override
    {
        assert(args);

        auto check = [&]()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_hello.find(L"hellohellohello") == std::wstring::npos;
        };


        while(check())
        {
           m_echoClient->echo(L"hello");
           std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        return true;
    }

    bool Exit(std::int32_t* exitCode) override
    {
        assert(exitCode);
        *exitCode = m_hello.find(L"hello") != std::wstring::npos ? 0 : 1;
        return true;
    }

    bool Finalize() override
    {
        m_echoServer.Hide();
        m_echo.deinit();
        return true;
    }

    bool CreateExecutor(std::shared_ptr<IExecutor>* executor) override
    {
        assert(executor);
        *executor = std::make_unique<Entries::SimpleExecutor>(*this);
        return true;
    }

    bool CreateProcessors(std::vector<std::unique_ptr<Gengine::ICmdProcessor>>* processors) override
    {
        assert(processors);
        return true;
    }

    bool echo(const std::wstring& hello) override
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_hello += hello;
        GLOG_INFO("responed");
        return true;
    }

private:
    ServiceObjectProxy<Echo::IEcho&> m_echoServer;
    ServiceClientProxy<Echo::IEcho> m_echoClient;
    EchoServer m_echo;

    std::mutex m_mutex;
    std::wstring m_hello;
};

REGISTER_MAIN_ENTRY(TestEcho)
IMPLEMENT_CONSOLE_ENTRY
