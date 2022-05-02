#include "ExecutableLauncher.h"
#include "Executable.h"

#if defined(_WIN32)
#include <Windows/ExecutableLauncherImpl.h>
#elif __linux__ || __APPLE__

#endif

#include <api/core/SessionId.h>
#include <core/Logger.h>

#include <thread>

namespace Gengine {
namespace InterprocessSynchronization {
using namespace Services;
using namespace AppConfig;

const std::uint32_t ExecutableLauncher::CheckTimeout(3000); // 3 sec
const std::uint32_t ExecutableLauncher::TryCounterMax(10);

ExecutableLauncher::ExecutableLauncher()
    : Worker(1)
    , m_sessionQuery("3514D35D")
    , m_checkAppsTimerId(INVALID_TIMER_ID)
{}

void ExecutableLauncher::AddExecutable(const executable_params& params, IExecutableLauncherListener& listener)
{
    auto handler = [this, params, &listener]() {
        if (m_executablesMap.empty())
        {
            Start();
        }
        m_executablesMap.emplace(params, listener);
    };
    GENGINE_POST_WAITED_TASK(handler);
}

void ExecutableLauncher::RemoveExecutable(const executable_params& params)
{
    auto handler = [this, params]() {
        WaitForStop(params);
        CheckDeadApps();
        m_executablesMap.erase(params);
        if (m_executablesMap.empty())
        {
            Stop();
        }
    };
    GENGINE_POST_WAITED_TASK(handler);
}

void ExecutableLauncher::WaitForStop(const executable_params& params)
{
    std::vector<std::shared_ptr<Executable>> deadApps;

    auto handler = [this, &deadApps, params]() {
        auto activeSessionKeys = GetActiveSessionKeys();
        for (const auto& sessionKey : activeSessionKeys)
        {
            auto key = std::make_pair(sessionKey, params);

            auto iter = m_executableInstances.find(key);
            if (iter != m_executableInstances.end())
            {
                deadApps.emplace_back(iter->second);
            }
        }
    };
    GENGINE_POST_WAITED_TASK(handler);

    std::uint32_t tryCounter = 0;
    auto dead = true;

    do
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        dead = true;
        for (const auto& guiApp : deadApps)
        {
            if (guiApp->IsAlive())
            {
                dead = false;
                break;
            }
        }
        ++tryCounter;
        if (tryCounter >= TryCounterMax)
        {
            GLOG_WARNING_INTERNAL("Dispose of gui apps failed. Killing...");
            for (const auto& appIter : deadApps)
            {
                appIter->Kill();
            }
            break;
        }
    } while (!dead);
}

std::vector<SessionId> ExecutableLauncher::GetActiveSessionKeys() const
{
    std::vector<SessionId> sessionIds;
    try
    {
        m_sessionQuery->QueryActiveSessionsIDs(&sessionIds);
    }
    catch (std::exception&)
    {
    }
    return sessionIds;
}

void ExecutableLauncher::StartInternal()
{
    if (m_checkAppsTimerId == INVALID_TIMER_ID)
    {
        auto handler = boost::bind(&ExecutableLauncher::CheckAppsRoutine, this);
        m_checkAppsTimerId = GENGINE_START_TIMER(handler, CheckTimeout);
    }
}

void ExecutableLauncher::StopInternal()
{
    if (m_checkAppsTimerId != INVALID_TIMER_ID)
    {
        GENGINE_STOP_TIMER_WITH_WAIT(m_checkAppsTimerId);
        m_checkAppsTimerId = INVALID_TIMER_ID;
    }

    for (const auto& executableIter : m_executablesMap)
    {
        WaitForStop(executableIter.first);
    }
}

void ExecutableLauncher::CheckAppsRoutine()
{
    CheckDeadApps();
    RunApps();
}

void ExecutableLauncher::CheckDeadApps()
{
    std::vector<std::shared_ptr<Executable>> deadApps;

    for (const auto& appIter : m_executableInstances)
    {
        auto& app = appIter.second;
        if (!app->IsAlive())
        {
            GLOG_INFO("Application %ls %ls died, schedule for remove.", app->GetParams().path, app->GetParams().params);
            deadApps.emplace_back(appIter.second);
        }
    }

    for (const auto& deadApp : deadApps)
    {
        OnExecutableClosed(deadApp);
        m_executablesMap.at(deadApp->GetParams()).OnExecutableClosed(deadApp);
        m_executableInstances.erase(std::make_pair(deadApp->GetSessionKey(), deadApp->GetParams()));
    }
}

void ExecutableLauncher::RunApps()
{
    auto activeSessionKeys = GetActiveSessionKeys();
    SessionId id; id.display = "vas";
    activeSessionKeys.push_back(id);
    for (const auto& sessionKey : activeSessionKeys)
    {
        for (const auto& executableIter : m_executablesMap)
        {
            auto executable = executableIter.first;
            auto& listener = executableIter.second;
            auto key = std::make_pair(sessionKey, executable);

            auto iter = m_executableInstances.find(key);
            if (iter == m_executableInstances.end())
            {
                auto appDesc = std::make_shared<Executable>(executable);
                if (appDesc)
                {
                    appDesc->Launch(sessionKey);
                    if (appDesc->IsAlive())
                    {
                        listener.OnExecutableLaunched(appDesc);
                        OnExecutableLaunched(appDesc);
                        m_executableInstances.emplace(std::make_pair(key, appDesc));
                    }
                }
            }
        }
    }
}

void ExecutableLauncher::OnExecutableLaunched(const std::shared_ptr<Executable>& app)
{

}

void ExecutableLauncher::OnExecutableClosed(const std::shared_ptr<Executable>& app)
{

}

bool ExecutableLauncher::IsCanStart()
{
    return true;
}

std::shared_ptr<ExecutableLauncher> CreateExecutableLauncher()
{
#if defined(_WIN32)
   return std::make_shared<ExecutableLauncherImpl>();
#elif __linux__ || __APPLE__
   return std::make_shared<ExecutableLauncher>();
#endif
}

}
}
