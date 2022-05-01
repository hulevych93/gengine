#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include <api/core/ISessionQuery.h>

namespace Gengine {
namespace TerminalSessions {
void GetActiveWTSSessionsIDs(std::vector<SessionId>& sessions);

}
}