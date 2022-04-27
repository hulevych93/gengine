#include "LinuxCallstackDumper.h"

#include <stdlib.h>
#include <stdio.h>
#include <execinfo.h>
#include <core/Logger.h>

namespace Gengine {
namespace Diagnostic {
LinuxCallstackDumper* LinuxCallstackDumper::m_instance = nullptr;

LinuxCallstackDumper::LinuxCallstackDumper()
{
    assert(m_instance == nullptr);
    m_instance = this;
    atexit(LinuxCallstackDumper::ExitHandler);
}

void LinuxCallstackDumper::WriteDump()
{
    int callStackSize = 0, callStackMaxSize = 256;
    void* callStackAddresses[callStackMaxSize];
    char** strings;

    callStackSize = backtrace(callStackAddresses, callStackMaxSize);
    GLOG_INFO("backtrace() returned %d addresses", callStackSize);

    strings = backtrace_symbols(callStackAddresses, callStackSize);
    if (strings)
    {
        std::string debugString= "\n";
        for(int j = 0; j < callStackSize; j++)
        {
            debugString += strings[j];
            debugString += "\n";
        }
        GLOG_INFO("%s", debugString.c_str());
        free(strings);
    }
}

void LinuxCallstackDumper::ExitHandler()
{
    m_instance->WriteDump();
}
}
}
