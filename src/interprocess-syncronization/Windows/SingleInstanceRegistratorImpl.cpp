#include "SingleInstanceRegistratorImpl.h"

#include <boost/lexical_cast.hpp>
#include <windows.h>

#include <core/Logger.h>

namespace Gengine {
namespace InterprocessSynchronization {
SingleInstanceRegistratorImpl::SingleInstanceRegistratorImpl(std::wstring&& objectName)
    : InstanceRegistratorInterface(std::move(objectName))
    , m_instanceMutex(nullptr)
{}

SingleInstanceRegistratorImpl::~SingleInstanceRegistratorImpl()
{
    if (IsInstanceRegistered())
    {
        UnregisterInstance();
    }
}

bool SingleInstanceRegistratorImpl::RegisterInstance()
{
    SetLastError(0);
    auto mutex = CreateMutexW(NULL, TRUE, GetObjectName().c_str());
    if (!mutex)
    {
        GLOG_ERROR("Failed create mutex error %d", GetLastError());
        assert(0);
        return false;
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        CloseHandle(mutex);
        return false;
    }
    m_instanceMutex = mutex;
    return true;
}

bool SingleInstanceRegistratorImpl::IsInstanceRegistered() const
{
    return m_instanceMutex != nullptr;
}

void SingleInstanceRegistratorImpl::UnregisterInstance()
{
    CloseHandle(reinterpret_cast<HANDLE>(m_instanceMutex));
    m_instanceMutex = nullptr;
}
}
}