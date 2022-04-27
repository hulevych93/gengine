#pragma once

#include <string>
#include <boost/signals2/connection.hpp>
#include <appconfig/AppConfig.h>

namespace Gengine {
namespace Services {

using connection = std::vector<boost::signals2::scoped_connection>;

class IProcessClient
{
public:
    virtual ~IProcessClient() = default;
    virtual void OnProcessLauched() = 0;
    virtual void OnProcessStopped() = 0;
};

class IProcessBroker
{
public:
    virtual ~IProcessBroker() = default;
    virtual void Configure(const std::set<ProcessConfig>& config) = 0;
    virtual void Deconfigure() = 0;
    virtual connection PowerUp(std::uint32_t id, IProcessClient& client) = 0;
    virtual void TearDown(std::uint32_t id, bool force = false) = 0;
};

class ProcessHolder: public IProcessClient
{
public:
    ProcessHolder(std::uint32_t id, IProcessClient * client = nullptr);
    ~ProcessHolder();

    void Run();
    void Stop();

protected:
    void OnProcessLauched() override;
    void OnProcessStopped() override;

private:
    const std::uint32_t m_id;
    connection endpoint;
    IProcessClient * m_listener;
};

void InitializeProcesses(const std::set<ProcessConfig>& config);
void DeinitializeProcesses();

#define INITIALIZE_PROCESSES(config) InitializeProcesses(config)
#define UNINITIALIZE_PROCESSES DeinitializeProcesses()

}
}