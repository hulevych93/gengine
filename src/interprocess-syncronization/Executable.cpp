#include "Executable.h"

#include <filesystem/Filesystem.h>

namespace Gengine {
namespace InterprocessSynchronization {
Executable::Executable(const executable_params& params)
    : m_params(params)
    , m_handle(nullptr)
    , m_pid(0)
{
}

void Executable::Launch(SessionId key)
{
    if (!IsAlive())
    {
#if 0
        process_handle_t info(nullptr);
        auto path = Filesystem::CombinePath(Filesystem::GetAppFolder(), m_params.path);
#if defined(BUILD_WINDOWS)
        if (ProcessHelper::LaunchProcessInSession(path, m_params.params, true, key.wtsId, info))
#elif __linux__ || __APPLE__
        if (ProcessHelper::LaunchProcessInSession(path, m_params.params, true, key.display, info))
#endif
        {
            m_handle = info.processHandle;
            m_pid = info.processId;
            m_key = key;
        }
#endif
    }
}

std::uint32_t Executable::GetPid() const
{
    return m_pid;
}

void* Executable::GetHandle() const
{
    return m_handle;
}

bool Executable::IsAlive()
{
    return m_pid > 0;
}

void Executable::Kill()
{

}

SessionId Executable::GetSessionKey() const
{
    return m_key;
}

const executable_params& Executable::GetParams() const
{
    return m_params;
}

}
}
