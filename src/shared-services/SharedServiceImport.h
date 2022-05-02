#pragma once

#include <core/AbstractFactory.h>
#include <shared-services/SharedServiceCreator.h>

namespace Gengine
{
template<class Interface, class Implementation, class... Args>
class LocalConcreteCreator : public IAbstractCreator<Interface, Args...>
{
public:
    std::shared_ptr<Interface> Create(Args... args) const override
    {
        return std::make_shared<Implementation>();
    }
};

}

#define IMRORT_SHARED_SERVICE(implementation) GENGINE_REGISTER_SERVICE("I" #implementation, (std::make_shared<LocalConcreteCreator<IMicroService, SymbolClient<I##implementation>, const std::string&>>()), implementation, ServiceType::Shared)

#define IMRORT_CREATOR_SHARED_SERVICE(implementation) GENGINE_REGISTER_SERVICE("I" #implementation, (std::make_shared<LocalConcreteCreator<IMicroService, CreationMethodClient<I##implementation>, const std::string&>>()), implementation, ServiceType::Shared)
