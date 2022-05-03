#pragma once

#include <string>
#include <memory>
#include <diagnostic\IDumper.h>

struct _CrtMemState;

namespace Gengine {
namespace Diagnostic {
class MemoryLeaksDumper : public IDumper
{
public:
    MemoryLeaksDumper(const std::string& file = "");
    ~MemoryLeaksDumper();

public:
    void WriteDump() override;

private:
    std::wstring GetDumpFilePath() const;

private:
    std::string m_fileName;
    void* m_handle;
    std::unique_ptr<_CrtMemState> m_memorySnapshot;
};
}
}