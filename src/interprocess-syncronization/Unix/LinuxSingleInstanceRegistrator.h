#pragma once

#include <interprocess-syncronization/InstanceRegistratorInterface.h>

namespace Gengine {
namespace InterprocessSynchronization {
class LinuxSingleInstanceRegistrator:  public InstanceRegistratorInterface
{
public:
    LinuxSingleInstanceRegistrator(std::wstring&& objectName);
    virtual ~LinuxSingleInstanceRegistrator();

    bool RegisterInstance() override;
    void UnregisterInstance() override;
    bool IsInstanceRegistered() const override;

private:
    int m_iSingleInstanceFile;
};
}
}
