#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <interprocess-syncronization/ServiceTracker.h>

namespace Gengine {
namespace InterprocessSynchronization {
class LinuxDaemonTracker: public ServiceTracker
{
public:
    LinuxDaemonTracker(const std::string& fileName, ServiceTrackerImpl& serviceTrackerImpl);
    ~LinuxDaemonTracker();

protected:
    void StartInternal() override;
    void StopInternal() override;
    bool IsCanStart() override;

private:
    std::thread m_thread;
    std::atomic<bool> m_canRun;
    void TrackerThreadProc();

private:
    std::string m_mappingFileName;
    int m_file;
};
}
}
