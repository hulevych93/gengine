#pragma once

#include <diagnostic/IDumper.h>
#include <setjmp.h>

namespace Gengine {
namespace Diagnostic {
class LinuxErrorsDumper : public IDumper {
 public:
  LinuxErrorsDumper();
  ~LinuxErrorsDumper() = default;

 public:
  void WriteDump() override;

 private:
  static int ErrorHandler(void*, void* evt);
  static int IOErrorHandler(void* d);

 private:
  static LinuxErrorsDumper* m_instance;
};
}  // namespace Diagnostic
}  // namespace Gengine
