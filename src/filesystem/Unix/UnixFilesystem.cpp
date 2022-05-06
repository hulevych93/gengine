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
