#include "DaemonAliveObject.h"

#include <unistd.h>
#include <fcntl.h>
#include <X11/Xlib.h>
#include <errno.h>
#include <cstring>

#include <core/Encoding.h>

namespace Gengine {
namespace InterprocessSynchronization {
DaemonAliveObject::DaemonAliveObject(const wchar_t *mappingFileName)
: m_aliveFile(-1)
{
    //lock file to inform GUI that we're alive
    //file will be automatically closed when this process exites
    m_aliveFile = open(toUtf8(mappingFileName).c_str(), O_CREAT | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);

    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 1;
    lock.l_type = F_WRLCK;
    fcntl(m_aliveFile, F_SETLK, &lock);
}


DaemonAliveObject::~DaemonAliveObject()
{
    Free();
}

void DaemonAliveObject::Free()
{
    if(IsLocked())
    {
        //
    }
}


bool DaemonAliveObject::IsLocked() const
{
    return m_aliveFile != -1;
}
}
}
