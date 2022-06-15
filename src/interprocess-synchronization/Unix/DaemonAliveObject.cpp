#include "DaemonAliveObject.h"

#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include <core/Encoding.h>

namespace Gengine {
namespace InterprocessSynchronization {
DaemonAliveObject::DaemonAliveObject(const std::wstring& mappingFileName)
    : m_aliveFile(-1) {
  m_aliveFile = socket(PF_LOCAL, SOCK_STREAM, 0);
  if (m_aliveFile != -1) {
    sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_LOCAL;
    strcpy(addr.sun_path, toUtf8(mappingFileName).c_str());

    unlink(toUtf8(mappingFileName).c_str());
    if (bind(m_aliveFile, (sockaddr*)&addr, SUN_LEN(&addr)) != -1) {
      if (listen(m_aliveFile, 20u) != -1) {
        return;
      }
    }
  }

  close(m_aliveFile);
  m_aliveFile = -1;
}

DaemonAliveObject::~DaemonAliveObject() {
  Free();
}

void DaemonAliveObject::Free() {
  if (IsLocked()) {
    close(m_aliveFile);
    m_aliveFile = -1;
  }
}

bool DaemonAliveObject::IsLocked() const {
  return m_aliveFile != -1;
}
}  // namespace InterprocessSynchronization
}  // namespace Gengine
