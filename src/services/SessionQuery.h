#pragma once

#include <api/core/ISessionQuery.h>

namespace Gengine {

class SessionQuery : public ISessionQuery {
 public:
  bool QueryActiveSessionsIDs(std::vector<SessionId>* ids) override;
};
}  // namespace Gengine