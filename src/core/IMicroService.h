#pragma once

#include <core/Export.h>
#include <services/ServicesCommon.h>

namespace Gengine {
namespace Services {
class GENGINE_API IMicroService {
 public:
  virtual ~IMicroService() = default;
};
}  // namespace Services
}  // namespace Gengine