#include <filesystem/Filesystem.h>

#include <dirent.h>
#include <dlfcn.h>
#include <fnmatch.h>
#include <sys/stat.h>
#include <unistd.h>

#include <core/Encoding.h>

namespace Gengine {
namespace Filesystem {

std::wstring GetExecutableFilePath() {
  return GetModuleFilePath(nullptr);
}

std::wstring GetModuleFilePath(void* /*handle*/) {
  char Buf[PATH_MAX + 1];
  ssize_t iRead = readlink("/proc/self/exe", Buf, PATH_MAX);
  if (iRead == (ssize_t)-1) {
    return std::wstring();
  }
  // add zero terminator
  Buf[iRead] = 0;

  return utf8toWchar(Buf);
}

}  // namespace Filesystem
}  // namespace Gengine
