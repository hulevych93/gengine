#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <api/core/ISessionQuery.h>

namespace Gengine {
namespace TerminalSessions {
void GetActiveWTSSessionsIDs(std::vector<SessionId>& sessions);

}
}  // namespace Gengine