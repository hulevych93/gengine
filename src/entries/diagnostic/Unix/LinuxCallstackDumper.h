#pragma once

#include <diagnostic/IDumper.h>
#include <memory>
#include <string>

namespace Gengine {
namespace Diagnostic {
class LinuxCallstackDumper : public IDumper {
 public:
  LinuxCallstackDumper();
  ~LinuxCallstackDumper();

 public:
  void WriteDump() override;

 private:
  static void ExitHandler();

 private:
  static LinuxCallstackDumper* m_instance;
};
}  // namespace Diagnostic
}  // namespace Gengine
