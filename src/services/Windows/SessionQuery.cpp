#include <services/SessionQuery.h>

#include <api/core/SessionId.h>
#include <services/Windows/WTSUtils.h>

namespace Gengine {

bool SessionQuery::QueryActiveSessionsIDs(std::vector<SessionId>* ids)
{
    assert(ids);

    TerminalSessions::GetActiveWTSSessionsIDs(*ids);
    return true;
}

}