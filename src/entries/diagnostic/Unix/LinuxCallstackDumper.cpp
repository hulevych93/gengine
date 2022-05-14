#include "LinuxCallstackDumper.h"

#include <core/Logger.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

namespace Gengine {
namespace Diagnostic {
LinuxCallstackDumper* LinuxCallstackDumper::m_instance = nullptr;

LinuxCallstackDumper::LinuxCallstackDumper() {
  assert(m_instance == nullptr);
  m_instance = this;
  atexit(LinuxCallstackDumper::ExitHandler);
}

LinuxCallstackDumper::~LinuxCallstackDumper() {
    m_instance = nullptr;
}

void LinuxCallstackDumper::WriteDump() {
  int callStackSize = 0, callStackMaxSize = 256;
  void* callStackAddresses[callStackMaxSize];
  char** strings;

  callStackSize = backtrace(callStackAddresses, callStackMaxSize);
  GLOG_INFO("backtrace() returned %d addresses", callStackSize);

  strings = backtrace_symbols(callStackAddresses, callStackSize);
  if (strings) {
    std::string debugString = "\n";
    for (int j = 0; j < callStackSize; j++) {
      debugString += strings[j];
      debugString += "\n";
    }
    GLOG_INFO("%s", debugString.c_str());
    free(strings);
  }
}

// If the ExitHandler is called after Gengine::Main class has been destructed,
// then it's not an emergency exit, it's a regular one. So, the m_instance is null
// in this case. No need to write a dump.
void LinuxCallstackDumper::ExitHandler() {
    if(m_instance)
    {
       m_instance->WriteDump();
    }
}
}  // namespace Diagnostic
}  // namespace Gengine
