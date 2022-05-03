#include <filesystem/Filesystem.h>

#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <core/Encoding.h>

namespace Gengine {
namespace Filesystem {

namespace fs = boost::filesystem;

bool IsFileExists(const std::wstring& file) {
  boost::system::error_code error;
  fs::path filePath(file);
  return fs::is_regular_file(filePath, error);
}

bool IsFolderExists(const std::wstring& folder) {
  boost::system::error_code error;
  fs::path folderPath(folder);
  return fs::is_directory(folderPath, error);
}

std::wstring GetFileName(const std::wstring& filePath) {
  fs::path path(filePath);
  return utf8toWchar(path.filename().string());
}

std::wstring GetFileNameWithoutExtension(const std::wstring& filePath) {
  auto name = GetFileName(filePath);
  const auto ext = name.rfind(L'.');
  if (ext != std::wstring::npos) {
    name = name.substr(0, ext);
  }
  return name;
}

std::wstring GetFilePath(const std::wstring& filePath) {
  fs::path path(filePath);
  return utf8toWchar(path.parent_path().string());
}

std::wstring CombinePath(const std::wstring& left, const std::wstring& right) {
  return utf8toWchar((fs::path(left) / fs::path(right)).string());
}

std::string CombinePath(const std::string& left, const std::string& right) {
  return (fs::path(left) / fs::path(right)).string();
}

std::wstring GetFileExtension(const std::wstring& filePath) {
  auto pos = filePath.find_last_of(L'.');
  if (pos == std::wstring::npos)
    return filePath;
  return filePath.substr(pos + 1);
}

std::wstring GetAppFolder() {
  return GetFilePath(GetExecutableFilePath());
}

std::wstring GetExecutableName() {
  return GetFileNameWithoutExtension(GetExecutableFilePath());
}

std::wstring GetModuleName(void* handle) {
  return GetFileNameWithoutExtension(GetModuleFilePath(handle));
}

bool DeleteEmptyFile(const std::wstring& szFilePath) {
  auto file = fs::path(szFilePath);
  if (!file.empty()) {
    boost::system::error_code code;
    if (fs::is_empty(file, code)) {
      if (fs::remove(file, code)) {
        return true;
      }
    }
  }
  return false;
}

bool CreateFolder(const std::wstring& path) {
  boost::system::error_code ec;
  boost::filesystem::create_directory(path, ec);
  return !ec.failed();
}

std::wstring GetRandomFileName(const std::wstring& szPattern) {
  wchar_t szBuf[1024];
  swprintf(szBuf, sizeof(szBuf) / sizeof(szBuf[0]), szPattern.c_str(), rand());
  std::wstring tmpName(szBuf);
  return tmpName;
}

}  // namespace Filesystem
}  // namespace Gengine
