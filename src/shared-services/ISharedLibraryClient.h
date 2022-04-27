#pragma once

#include <core/IMicroService.h>

namespace Gengine {
namespace SharedServices {

class ISharedLibraryClient
{
public:
    virtual ~ISharedLibraryClient() = default;
    virtual std::shared_ptr<Services::IMicroService> Connect(const SharedConnection& data) = 0;
};

}
}
