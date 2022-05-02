#pragma once

#include <string>

namespace Gengine {
namespace Filesystem {

bool IsFileExists(const std::wstring& path);
bool IsFolderExists(const std::wstring& path);
bool CreateFolder(const std::wstring& lpsPath);
bool DeleteExistingFile(const std::wstring& path);

std::wstring GetFileName(const std::wstring& filePath);
std::wstring GetFileNameWithoutExtension(const std::wstring& filePath);
std::wstring GetFilePath(const std::wstring& filePath);
std::wstring GetFileExtension(const std::wstring& fileName);

std::wstring CombinePath(const std::wstring& left, const std::wstring& right);
std::string CombinePath(const std::string& left, const std::string& right);

std::wstring GetAppFolder();
std::wstring GetSystemFolder();
std::wstring GetExecutableName();
std::wstring GetTempDirPath();
std::wstring GetSystemDirPath();
std::wstring GetModuleName(void* handle);
std::wstring GetExecutableFilePath();
std::wstring GetModuleFilePath(void* handle);
std::wstring GetKernelObjectPath(const std::wstring& name);

std::wstring GetRandomFileName(const std::wstring& pattern);
}
}
