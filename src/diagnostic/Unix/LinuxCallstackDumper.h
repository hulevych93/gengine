#pragma once

#include <string>
#include <memory>
#include <diagnostic/IDumper.h>

namespace Gengine {
namespace Diagnostic {
class LinuxCallstackDumper: public IDumper
{
public:
    LinuxCallstackDumper();
    ~LinuxCallstackDumper() = default;

public:
    void WriteDump() override;

private:
    static void ExitHandler();

private:
    static LinuxCallstackDumper *m_instance;
};
}
}
