#pragma once

#include <atomic>
#include <boost/signals2.hpp>
#include <api/core/IRunnable.h>

namespace Gengine {
class Runnable: public IRunnable
{
public:
    using connection = boost::signals2::connection;
    using signal = boost::signals2::signal<void()>;

public:
    Runnable();

    bool Start() override;
    bool IsRunning(bool* running) override;
    bool Stop() override;

    connection AddStartedListener(signal::slot_function_type slot);
    connection AddStoppedListener(signal::slot_function_type slot);

protected:
    virtual void StartInternal() = 0;
    virtual void StopInternal() = 0;
    virtual bool IsCanStart();

private:
    std::atomic<bool> m_isRunning;

private:
    signal m_startedSignal;
    signal m_stoppedSignal;
};
}
