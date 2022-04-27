#pragma once

#include <interprocess-syncronization/IAliveObject.h>

namespace Gengine {
namespace InterprocessSynchronization {
class DaemonAliveObject: public IAliveObject
{
public:
    explicit DaemonAliveObject(const wchar_t* mappingFileName);
    ~DaemonAliveObject();

    void Free() override;
    bool IsLocked() const override;

private:
    int m_aliveFile;
};
}
}
