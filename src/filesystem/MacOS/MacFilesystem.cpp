#include <filesystem/Filesystem.h>

#include <dlfcn.h>
#include <dirent.h>
#include <fnmatch.h>
#include <mach-o/dyld.h>
#include <sys/stat.h>
#include <unistd.h>

#include <core/Encoding.h>

namespace Gengine {
namespace Filesystem {

std::wstring GetExecutableFilePath() {
  std::string buf(1024, 0);
  uint32_t size = static_cast<uint32_t>(buf.size());
  int result = _NSGetExecutablePath(&buf[0], &size);
  if (result == -1) {
    buf.resize(size + 1);
    std::fill(std::begin(buf), std::end(buf), 0);
  }
  return utf8toWchar(buf);
}

std::wstring GetModuleFilePath(void* handle) {
  Dl_info info;
  if (dladdr(handle, &info)) {
    return utf8toWchar(info.dli_fname);
  }

  return {};
}

}  // namespace Filesystem
}  // namespace Gengine
