#include "LinuxDaemonTracker.h"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <string.h>
#include <unistd.h>

#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <core/Encoding.h>
#include <core/Logger.h>

namespace Gengine {
namespace InterprocessSynchronization {
LinuxDaemonTracker::LinuxDaemonTracker(const std::string& fileName,
                                       terminate_handler handler)
    : ServiceTracker(handler), m_mappingFileName(fileName), m_aliveFile(-1) {
  if (pipe(m_selfSignalPipe) == -1) {
    throw std::runtime_error("can't construct daemon tracker");
  }
  ::fcntl(m_selfSignalPipe[0], F_SETFL, O_NONBLOCK);
  ::fcntl(m_selfSignalPipe[1], F_SETFL, O_NONBLOCK);
}

LinuxDaemonTracker::~LinuxDaemonTracker() {
  Stop();
  close(m_selfSignalPipe[0]);
  close(m_selfSignalPipe[1]);
  if (m_aliveFile != -1) {
    close(m_aliveFile);
    m_aliveFile = -1;
  }
}

void LinuxDaemonTracker::StartInternal() {
  if (!m_thread.joinable()) {
    m_thread = std::thread(&LinuxDaemonTracker::RunLoop, this);
  }
}

void LinuxDaemonTracker::StopInternal() {
  if (m_thread.joinable()) {
    write(m_selfSignalPipe[1], "", 1);
    m_thread.join();
  }
}

bool LinuxDaemonTracker::IsCanStart() {
  m_aliveFile = socket(PF_LOCAL, SOCK_STREAM, 0);
  if (m_aliveFile != -1) {
    sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_LOCAL;
    strcpy(addr.sun_path, m_mappingFileName.c_str());

    if (connect(m_aliveFile, (sockaddr*)&addr, SUN_LEN(&addr)) != -1) {
      return true;
    }
    close(m_aliveFile);
    m_aliveFile = -1;
  }
  return false;
}

void LinuxDaemonTracker::RunLoop() {
  pollfd fds[2];

  fds[0].fd = m_aliveFile;
  fds[0].events = POLLIN;

  fds[1].fd = m_selfSignalPipe[0];
  fds[1].events = POLLIN;

  while (true) {
    if (poll(fds, 2, -1) > 0) {
      if ((fds[0].revents & POLLERR) || (fds[0].revents & POLLIN)) {
        GLOG_INFO("Daemon died! Exiting...");
        ServiceTracker::Terminate();
        return;
      }

      if (fds[1].revents & POLLIN) {
        break;
      }
    }
  }
}
}  // namespace InterprocessSynchronization
}  // namespace Gengine
