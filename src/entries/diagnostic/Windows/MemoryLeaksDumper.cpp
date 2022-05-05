#include "MemoryLeaksDumper.h"

#include <core/Encoding.h>
#include <filesystem/Filesystem.h>
#include <windows.h>

namespace Gengine {
namespace Diagnostic {
MemoryLeaksDumper::MemoryLeaksDumper(const std::string& name)
    : m_fileName(name),
      m_handle(nullptr),
      m_memorySnapshot(std::make_unique<_CrtMemState>()) {
  auto filePath = GetDumpFilePath();
  if (!filePath.empty()) {
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    auto hFile =
        ::CreateFileW(filePath.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE,
                      nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
      _CrtSetReportFile(_CRT_WARN, hFile);
      m_handle = hFile;
    }
  } else {
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
  }

  _CrtMemCheckpoint(&(*m_memorySnapshot.get()));
}

MemoryLeaksDumper::~MemoryLeaksDumper() {
  WriteDump();
  if (m_handle != nullptr) {
    ::CloseHandle(reinterpret_cast<HANDLE>(m_handle));
  }
}

void MemoryLeaksDumper::WriteDump() {
  _CrtMemDumpAllObjectsSince(&(*m_memorySnapshot.get()));
}

std::wstring MemoryLeaksDumper::GetDumpFilePath() const {
  if (!m_fileName.empty()) {
    auto fileName = m_fileName + ".memory";
    if (!fileName.empty()) {
      return Filesystem::CombinePath(Filesystem::GetAppFolder(),
                                     utf8toWchar(fileName));
    }
  }

  return std::wstring();
}
}  // namespace Diagnostic
}  // namespace Gengine