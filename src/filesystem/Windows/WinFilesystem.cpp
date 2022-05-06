#include <filesystem/Filesystem.h>

#include <Shlobj.h>
#include <Windows.h>
#include <direct.h>
#include <io.h>
#include <shellapi.h>

#include <core/Encoding.h>
#include <core/Logger.h>

namespace Gengine {
namespace Filesystem {

std::wstring GetSystemFolder() {
  std::wstring result;

  wchar_t path[MAX_PATH];
  SHGetSpecialFolderPathW(0, path, CSIDL_PROGRAM_FILES_COMMON, FALSE);
  result = path;

  return result;
}

std::wstring GetModuleFilePath(void* handle) {
  wchar_t sPath[500];

  memset(sPath, 0, 500 * sizeof(TCHAR));

  int nRet =
      ::GetModuleFileNameW(reinterpret_cast<HMODULE>(handle), sPath, 500);

  assert(nRet);

  std::wstring strPath(sPath);
  // some system processes have such leading characters in image path
  //(e.g. Winlogon.exe where we have our resident)
  if (strPath.find(L"\\??\\") == 0)
    strPath = strPath.substr(4);

  return strPath;
}

std::wstring GetExecutableFilePath() {
  return GetModuleFilePath(nullptr);
}

bool DeleteExistingFile(const std::wstring& strFilePath) {
  return DeleteFileW(strFilePath.c_str()) != FALSE;
}

std::wstring GetTempDirPath() {
  DWORD dwBufSize = 256;
  wchar_t* pBuf = new wchar_t[dwBufSize];
  DWORD dwRequired = GetTempPathW(dwBufSize - 1, pBuf);
  if (dwRequired > dwBufSize) {  // allocate enough space and try again
    delete[] pBuf;
    dwBufSize = dwRequired + 1;
    pBuf = new wchar_t[dwBufSize];
    GetTempPathW(dwBufSize - 1, pBuf);
  }
  std::wstring strResult = pBuf;
  delete[] pBuf;
  return strResult;
}

std::wstring GetSystemDirPath() {
  auto systemFolderBuf = std::make_unique<wchar_t[]>(MAX_PATH);
  GetSystemDirectoryW(systemFolderBuf.get(), MAX_PATH);
  return std::wstring(systemFolderBuf.get());
}

std::wstring GetKernelObjectPath(const std::wstring& name) {
  return std::wstring{L"\\Global\\\\"} + name;
}

}  // namespace Filesystem
}  // namespace Gengine
