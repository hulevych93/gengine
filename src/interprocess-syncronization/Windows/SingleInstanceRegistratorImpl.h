#pragma once

#include <interprocess-syncronization/InstanceRegistratorInterface.h>

namespace Gengine {
namespace InterprocessSynchronization {
class SingleInstanceRegistratorImpl : public InstanceRegistratorInterface
{
public:
    explicit SingleInstanceRegistratorImpl(std::wstring&& strObjectName);
    virtual ~SingleInstanceRegistratorImpl();

    bool RegisterInstance() override;
    void UnregisterInstance() override;
    bool IsInstanceRegistered() const override;

private:
    void* m_instanceMutex;
};
}
}