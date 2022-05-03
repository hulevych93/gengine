#include <services/SessionQuery.h>

#include <api/core/SessionId.h>

namespace Gengine {

bool SessionQuery::QueryActiveSessionsIDs(std::vector<SessionId>* ids) {
  assert(ids);
  return true;
}

}  // namespace Gengine
