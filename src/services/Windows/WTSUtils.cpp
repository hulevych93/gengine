#include "WTSUtils.h"

#include <Windows.h>
#include <WtsApi32.h>

#include <core/Logger.h>

#include <api/core/SessionId.h>

namespace Gengine {
namespace TerminalSessions {

void GetActiveWTSSessionsIDs(std::vector<SessionId>& sessions)
{
    WTS_SESSION_INFO* sessionsInfo(NULL);
    DWORD count = 0;
    if (!WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &sessionsInfo, &count))
    {
        DWORD err = GetLastError();
        if (err == RPC_S_INVALID_BINDING)
        {
            return;
        }
        else
        {
            GLOG_ERROR("GetAllWTSSessions failed, error code == %d", err);
            return;
        }
    }
    for (DWORD i = 0; i < count; ++i)
    {
        SessionId id;
        id.wtsId = static_cast<std::uint32_t>(sessionsInfo[i].SessionId);
        sessions.push_back(id);
    }

    WTSFreeMemory(sessionsInfo);
}

}
}