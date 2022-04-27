#pragma once

#include <memory>
#include <interprocess-communication/InterprocessCommonDefs.h>

namespace Gengine {
namespace Services {
class IMicroService;
}
namespace InterprocessCommunication {
class InputParameters;
class OutputParameters;

class InterfaceImpl
{
public:
    virtual ~InterfaceImpl() = default;
    virtual interface_key GetInterface() const = 0;
};

class InterfaceListener: public InterfaceImpl
{
public:
    virtual ~InterfaceListener() = default;
    virtual ResponseCodes HandleEvent(std::uint8_t function,
        const std::shared_ptr<const InputParameters>& inputs) = 0;
};

class InterfaceExecutor : public InterfaceListener
{
public:
    virtual ~InterfaceExecutor() = default;
    virtual ResponseCodes HandleRequest(std::uint8_t function,
        const std::shared_ptr<const InputParameters>& inputs,
        const std::shared_ptr<OutputParameters>& outputs) = 0;
};

using interface_object = boost::variant<std::shared_ptr<InterfaceListener>, std::shared_ptr<InterfaceExecutor>>;
}
}