#include <filesystem/Filesystem.h>

#include <dlfcn.h>
#include <fstream>

#include <dirent.h>
#include <fnmatch.h>
#include <mach-o/dyld.h>
#include <sys/stat.h>
#include <unistd.h>

#include <core/Encoding.h>

namespace Gengine {
namespace Filesystem {

bool DeleteExistingFile(const std::wstring& strFilePath) {
  return unlink(toUtf8(strFilePath).c_str()) == 0;
}

std::wstring GetSystemFolder() {
  return L"/etc";
}

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

bool DeleteEmptyFolder(const std::wstring& szFolderPath) {
  return rmdir(toUtf8(szFolderPath).c_str()) == 0;
}

std::wstring GetTempDirPath() {
  return L"/tmp/";
}

std::wstring GetSystemDirPath() {
  return L"/var/";
}

std::wstring GetKernelObjectPath(const std::wstring& name) {
  return std::wstring{L"/tmp/"} + name;
}

}  // namespace Filesystem
}  // namespace Gengine
