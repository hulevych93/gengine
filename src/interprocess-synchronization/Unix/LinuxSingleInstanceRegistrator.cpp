#include "LinuxSingleInstanceRegistrator.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <core/Encoding.h>
#include <core/Logger.h>

namespace Gengine {
namespace InterprocessSynchronization {
LinuxSingleInstanceRegistrator::LinuxSingleInstanceRegistrator(
    std::wstring&& objectName)
    : InstanceRegistratorInterface(std::move(objectName)), m_file(-1) {}

LinuxSingleInstanceRegistrator::~LinuxSingleInstanceRegistrator() {
  if (IsInstanceRegistered()) {
    UnregisterInstance();
  }
}

bool LinuxSingleInstanceRegistrator::RegisterInstance() {
  // we may run under the root, set mode of files to 777
  // to allow anybody other run after us
  umask(0000);
  if (m_file != -1)
    return true;
  int lock_file = open(toUtf8(GetObjectName()).c_str(), O_CREAT | O_WRONLY,
                       S_IRWXU | S_IRWXG | S_IRWXO);
  if (lock_file == -1) {
    return false;
  }

  char uDummy = 'x';
  write(lock_file, &uDummy, 1);
  struct flock lock;
  memset(&lock, 0, sizeof(lock));
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = 1;
  lock.l_type = F_WRLCK;
  if (fcntl(lock_file, F_SETLK, &lock) == -1) {
    close(lock_file);
    return false;
  }
  m_file = lock_file;
  return true;
}

void LinuxSingleInstanceRegistrator::UnregisterInstance() {
  if (m_file == -1)
    return;
  struct flock lock;
  memset(&lock, 0, sizeof(lock));
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = 1;
  lock.l_type = F_UNLCK;
  fcntl(m_file, F_SETLK, &lock);
  close(m_file);
  m_file = -1;
}

bool LinuxSingleInstanceRegistrator::IsInstanceRegistered() const {
  return m_file != -1;
}
}  // namespace InterprocessSynchronization
}  // namespace Gengine
