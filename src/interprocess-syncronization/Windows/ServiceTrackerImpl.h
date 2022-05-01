#pragma once

#include <thread>
#include <interprocess-syncronization/ServiceTracker.h>

namespace Gengine {
namespace InterprocessSynchronization {
class ServiceTrackerImpl : public ServiceTracker
{
public:
    ServiceTrackerImpl(const std::wstring& mappingFileName, ServiceTracker::terminate_handler handler);
    virtual ~ServiceTrackerImpl();

protected:
    void StartInternal() override;
    void StopInternal() override;
    bool IsCanStart() override;

private:
    std::unique_ptr<std::thread> m_thread;
    void TrackerThreadProc();

private:
    std::wstring m_mappingFileName;
    void* m_fileMappingHandle;
    void* m_fileMappingData;
    Multithreading::Event m_stopEvent;
};
}
}