#pragma once

#include <memory>

template<class Interface, class... Args>
class IAbstractCreator
{
public:
    virtual std::shared_ptr<Interface> Create(Args... args) const = 0;
};

