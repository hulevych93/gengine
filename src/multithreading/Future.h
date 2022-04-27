#pragma once

#include <cstdint>
#include <atomic>
#include <multithreading/Event.h>
#include <brokers/WorkerBroker.h>

namespace Gengine {
namespace Multithreading {
class Future: public Services::IFuture
{
public:
    explicit Future(bool processing);
    virtual ~Future();

    void Wait(std::uint32_t timeout = Event::WAIT_INFINITE) override;
    bool IsCanceled() const override;
    void Cancel() override;
    void Complete() override;
    void Reset() override;

private:
    Event m_completed;
    std::atomic<bool> m_canceled;
    std::atomic<bool> m_processing;
};
}
}
