#pragma once

#include <string>
#include <memory>

namespace Gengine {
namespace InterprocessSynchronization {
enum class InstanceType
{
    Sigle,
    OnePerUserSession
};

class InstanceRegistratorInterface
{
public:
    virtual ~InstanceRegistratorInterface() = default;
    virtual bool RegisterInstance() = 0;
    virtual void UnregisterInstance() = 0;
    virtual bool IsInstanceRegistered() const = 0;

protected:
    explicit InstanceRegistratorInterface(std::wstring&& objectName);
    const std::wstring& GetObjectName() const;

private:
    std::wstring m_objectName;
};
}
}