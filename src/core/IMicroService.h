#pragma once

#include <services/ServicesCommon.h>
#include <core/Export.h>

namespace Gengine {
namespace Services {
class GENGINE_API IMicroService
{
public:
    virtual ~IMicroService() = default;
};
}
}