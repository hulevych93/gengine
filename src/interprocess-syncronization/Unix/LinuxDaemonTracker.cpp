#include "LinuxDaemonTracker.h"

#include <X11/Xlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <core/Encoding.h>
#include <core/Logger.h>

namespace Gengine {
namespace InterprocessSynchronization {
LinuxDaemonTracker::LinuxDaemonTracker(const std::string& fileName,
                                       terminate_handler handler)
    : ServiceTracker(handler),
      m_canRun(false),
      m_mappingFileName(fileName),
      m_file(-1) {}

LinuxDaemonTracker::~LinuxDaemonTracker() {
  Stop();
}

void LinuxDaemonTracker::StartInternal() {
  if (!m_thread.joinable()) {
    m_canRun.store(true);
    m_thread = std::thread(&LinuxDaemonTracker::TrackerThreadProc, this);
  }
}

void LinuxDaemonTracker::StopInternal() {
  if (m_thread.joinable()) {
    m_canRun.store(false);
    m_thread.join();
  }
}

bool LinuxDaemonTracker::IsCanStart() {
  m_file = open(m_mappingFileName.c_str(), O_CREAT | O_WRONLY,
                S_IRWXU | S_IRWXG | S_IRWXO);
  return m_file != -1;
}

void LinuxDaemonTracker::TrackerThreadProc() {
  while (m_canRun.load()) {
    // try to lock file
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 1;
    lock.l_type = F_WRLCK;

    // check daemon death
    if (fcntl(m_file, F_SETLK, &lock) != -1) {
      GLOG_INFO("Service died! Exiting...");
      // lock successfully acuired;
      // this means, that daemon died
      ServiceTracker::Terminate();
      close(m_file);
    }

    usleep(100000);
  }
}
}  // namespace InterprocessSynchronization
}  // namespace Gengine
