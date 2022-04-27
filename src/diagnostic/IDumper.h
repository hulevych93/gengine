#pragma once

namespace Gengine {
namespace Diagnostic {
class IDumper
{
public:
    virtual ~IDumper() = default;
    virtual void WriteDump() = 0;
};
}
}