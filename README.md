# gengine
This is a modern C++14 and scalable cross-platfrom application development framework

[![Windows](https://github.com/hulevych93/gengine/actions/workflows/Windows.yml/badge.svg)](https://github.com/hulevych93/gengine/actions/workflows/Windows.yml)
[![Linux](https://github.com/hulevych93/gengine/actions/workflows/Linux.yml/badge.svg)](https://github.com/hulevych93/gengine/actions/workflows/Linux.yml)
[![Apple MacOS](https://github.com/hulevych93/gengine/actions/workflows/MacOS.yml/badge.svg)](https://github.com/hulevych93/gengine/actions/workflows/MacOS.yml)

It includes such common development tasks solutions:
* Windows service and unix daemons development.
* Child-parent process management.
* Inteprocess communication via local sockets or TCP/IP.
* Custom interface definition language configuration files.
* Possibility to link interfaces and implementations via custom *.json files configs.  

## How to build?

* Install perl. 
* Setup BUILD_FILES_ROOT environment variable for 3rd parties build.
* Run cmake command to setup build environment.

### Windows
The gengine is built with v141 toolset at the moment. Here is cmake command line for @Visual Studio 2017

```cmake
cmake -G "Visual Studio 15 2017" -T "v141" ${PATH_TO_GENGINGE}
cmake -G "Visual Studio 15 2017 Win64" -T "v141" ${PATH_TO_GENGINGE}
```

### Unix

As well gengine is build with clang compiler on unix like OS.

```cmake
cmake -G "Unix Makefiles" -DCMAKE_C_COMPILER=${PATH_TO_CLANG} -DCMAKE_CXX_COMPILER=${PATH_TO_CLANG++} ${PATH_TO_GENGINGE}
```

## How could be used?

### Example #1
Easy to run application as background platform process.
```c++
#include <gengine/gengine.h>

using namespace Gengine;
using namespace Gengine::Services;
using namespace Gengine::Entries;

class TestService final : public Entries::EntryBase,
                          public Worker
{
public:
    TestService(std::unique_ptr<IEntryToolsFactory>&& factory)
        : EntryBase(std::move(factory))
        , Worker(0)
    {

    }

    ~TestService() = default;

    bool Initialize() override
    {
        // init application
        return true;
    }

    bool Execute(void* args) override
    {
        assert(args);
        // run application
        return true;
    }

    bool Exit(std::int32_t* exitCode) override
    {
        assert(exitCode);
        *exitCode = 0;
        return true;
    }

    bool Finalize() override
    {
        // finalize application
        return true;
    }

    bool CreateExecutor(std::shared_ptr<IExecutor>* executor) override
    {
        assert(executor);
        
        // the process will be run as unix daemon or windows service
        *executor = makeServiceExecutor(*this);
        return true;
    }

    bool CreateProcessors(std::vector<std::unique_ptr<Gengine::ICmdProcessor>>* processors) override
    {
        assert(processors);
        // the app has no command line processor
        return true;
    }
};

REGISTER_MAIN_ENTRY(TestService)
IMPLEMENT_CONSOLE_ENTRY

```

### Example 2

The echo client server app.

The client server api via *.rdl file
```
[Dir(echo)]
namespace Echo
{
    interface IEcho
    {
        echo([in]WString hello);
    };
};
```

Client server applicaiton looks like this
```c++

//...

using namespace Gengine;
using namespace Gengine::Services;
using namespace Gengine::Entries;

class EchoServer : public Echo::IEcho,
                   Worker
{
public:
    EchoServer()
        : Worker(200) // worker thread id
        , m_echoClient("3E0474C1") // service id in *.json config
        , m_echoServer("83F52161", *this)
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
        GENGINE_POST_TASK(handler); // post task via worker thread
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
        , m_echoClient("83F52161")
        , m_echoServer("3E0474C1", *this)
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

        while(true)
        {
           m_echoClient->echo(L"hello");
           std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        return true;
    }

    bool Exit(std::int32_t* exitCode) override
    {
        assert(exitCode);
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
        GLOG_INFO("responed: %s", hello);
        return true;
    }

private:
  // ...
};

REGISTER_MAIN_ENTRY(TestEcho)
IMPLEMENT_CONSOLE_ENTRY

```

The endpoint is provided to *.exe file via self-packed json.

```json
{
  "appName": "TestEcho",
  "consoleLogger": true,
  "workers": [
    {
      "id": 0,
      "name": "MainTestThread"
    }
  ],
  "inServices": [
    {
      "comment" : "IEcho service is connected via tcp/ip"
      "key": "A2H11HJK",
      "value": {
        "id": "A2H11HJK",
        "key": "IEcho",
        "type": 1,
        "connection": {
          "which": 2,
          "data": {
            "which": 1,
            "data": {
              "ip": "127.0.0.1",
              "port": 35009
            }
          }
        }
      }
    }
  ],
  "outServices": [
  {
    "comment" : "IEcho server running on pipe or socket"
    "key": "3E0474C1",
    "value": {
      "id": "3E0474C1",
      "key": "IEcho",
      "type": 1,
      "connection": {
        "which": 2,
        "data": {
          "which": 0,
          "data": {
            "pipe": "D7C9AA6F49D5"
          }
        }
      },
      "comment" : "thread where server is running"
      "threadRef": 195 
    }
  }
  ]
}
 
```
