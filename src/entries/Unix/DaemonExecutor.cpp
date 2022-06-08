#include <entries/Unix/DaemonExecutor.h>

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>

#include <execinfo.h>
#include <boost/format.hpp>

#include <api/entries/ICmdProcessor.h>
#include <api/entries/IEntry.h>
#include <core/Logger.h>

namespace Gengine {
namespace Entries {

volatile sig_atomic_t DaemonExecutor::m_terminationInProgress = 0;
DaemonExecutor* DaemonExecutor::m_instance = nullptr;
volatile sig_atomic_t DaemonExecutor::m_handlingSignal = 0;

DaemonExecutor::DaemonExecutor(IEntry& entry)
    : SimpleExecutor(entry),
      m_canHandleSignals(false),
      m_lastSigNum(0),
      m_crashBuffSize(200),
      m_crashAddressesNum(0) {
  m_instance = this;
  m_stopEvent.Create(true, false);
}

void DaemonExecutor::StartSignalThread() {
  if (!m_signalHandlerThread.joinable()) {
    sem_init(&m_signalSemaphore, 0, 0);
    m_canHandleSignals.store(true);
    m_signalHandlerThread =
        std::thread(std::bind(&DaemonExecutor::SignalHandlingRoutine, this));
  }
}

void DaemonExecutor::StopSignalThread() {
  if (m_signalHandlerThread.joinable()) {
    m_canHandleSignals.store(false);
    sem_post(&m_signalSemaphore);
    m_signalHandlerThread.join();
  }
}

void DaemonExecutor::SignalHandlingRoutine() {
  while (m_canHandleSignals.load()) {
    sem_wait(&m_signalSemaphore);

    if (!m_canHandleSignals) {
      break;
    }

    switch (m_lastSigNum) {
      case SIGSEGV:
      case SIGABRT:
        OnCrash(m_lastSigNum);
        break;
      case SIGTERM:
        OnTerm();
        break;
    }
  }
}

void DaemonExecutor::SignalHandler(int signum) {
  if (m_instance) {
    m_instance->OnSignal(signum);
  }
}

void DaemonExecutor::OnSignal(int signum) {
  if (m_terminationInProgress) {
    return;
  }
  switch (signum) {
    case SIGSEGV:
    case SIGABRT: {
      m_terminationInProgress = 1;

      m_lastSigNum = signum;

      m_crashAddressesNum = backtrace(m_crashBuff, m_crashBuffSize);

      sem_post(&m_signalSemaphore);

      // we do not want continue execution
      exit(EXIT_FAILURE);
    } break;
    case SIGALRM:
      exit(EXIT_FAILURE);
      break;
    case SIGUSR1:
    case SIGCHLD:
      _exit(1);
    case SIGTERM:
      m_terminationInProgress = 1;

      m_lastSigNum = signum;

      sem_post(&m_signalSemaphore);
      break;
  }
}

void DaemonExecutor::OnTerm() {
  m_stopEvent.Set();
  _exit(1);
}

void DaemonExecutor::OnCrash(int signum) {
  char** strings;

  strings = backtrace_symbols(m_crashBuff, m_crashAddressesNum);
  if (strings == NULL) {
    GLOG_INFO("backtrace_symbols failed, error == %d", errno);
    /*        perror("backtrace_symbols");
            exit(EXIT_FAILURE);*/
  } else {
    std::string debugString = "\n";
    for (int j = 0; j < m_crashAddressesNum; j++) {
      debugString += boost::str(boost::format("%s\n") % strings[j]);
    }
    GLOG_INFO("%s", debugString.c_str());
    free(strings);
  }

  // 2 sec additional sleep. seems dumper async or smth.
  usleep(2000 * 1000);

  signal(signum, SIG_DFL);
  raise(signum);
}

bool DaemonExecutor::Execute(void* args) {
  m_crashBuffSize = 200;
  m_lastSigNum = 0;
  m_crashAddressesNum = 0;

  pid_t pid, sid, parent;

  /* already a daemon */
  if (getppid() == 1) {
    // We're already a daemon. No actions performed
    return true;
  }

  /* Create the lock file as the current user */
  /*if (lockfile && lockfile[0]) {
      lfp = open(lockfile, O_RDWR | O_CREAT, 0640);
      if (lfp < 0) {
          syslog(LOG_ERR, "unable to create lock file %s, code=%d (%s)",
                 lockfile, errno, strerror(errno));
          exit(EXIT_FAILURE);
      }
  }*/

  /* Drop user if there is one, and we were run as root */
  /*if (getuid() == 0 || geteuid() == 0)
  {
      struct passwd *pw = getpwnam(RUN_AS_USER);
      if (pw) {
          syslog(LOG_NOTICE, "setting user to " RUN_AS_USER);
          setuid(pw->pw_uid);
      }
  }*/

  /* Trap signals that we expect to receive */
  signal(SIGCHLD, SignalHandler);
  signal(SIGUSR1, SignalHandler);
  signal(SIGALRM, SignalHandler);
  signal(SIGTERM, SignalHandler);
  signal(SIGSEGV, SignalHandler);
  signal(SIGABRT, SignalHandler);

  /* Fork off the parent process */
  pid = fork();

  if (pid < 0) {
    std::cerr << "unable to fork daemon, code " << errno << strerror(errno)
              << std::endl;
    return false;
  }
  /* If we got a good PID, then we can exit the parent process. */
  if (pid > 0) {
    std::cout << "Forked child process..." << std::endl;
    /* Wait for confirmation from the child via SIGTERM or SIGCHLD, or
       for two seconds to elapse (SIGALRM).  pause() should not return. */
    alarm(2);
    pause();
    return true;
  }

  std::this_thread::sleep_for(std::chrono::seconds(25));

  StartSignalThread();

  /* At this point we are executing as the child process */
  parent = getppid();

  /* Cancel certain signals */
  signal(SIGCHLD, SIG_DFL); /* A child process dies */
  signal(SIGTSTP, SIG_IGN); /* Various TTY signals */
  signal(SIGTTOU, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGHUP, SIG_IGN); /* Ignore hangup signal */

  /* Change the file mode mask */
  umask(0);

  /* Create a new SID for the child process */
  sid = setsid();
  if (sid < 0) {
    std::cerr << "unable to create a new session, code " << errno
              << strerror(errno) << std::endl;
    return false;
  }

  /* Change the current working directory.  This prevents the current
     directory from being locked; hence not being able to remove it. */
  if ((chdir("/")) < 0) {
    std::cerr << "unable to change directory, code " << errno << strerror(errno)
              << std::endl;
    return false;
  }

  /* Redirect standard files to /dev/null */
  freopen("/dev/null", "r", stdin);
  freopen("/dev/null", "w", stdout);
  freopen("/dev/null", "w", stderr);

  kill(parent, SIGUSR1);

  if (GetEntry().Initialize()) {
    GetEntry().Execute(args);
    m_stopEvent.Wait(-1);

    std::int32_t code;
    GetEntry().Exit(&code);
    GetEntry().Finalize();
  }

  return true;
}

bool DaemonExecutor::CreateProcessors(
    std::vector<std::unique_ptr<ICmdProcessor>>* processors) {
  return true;
}

std::unique_ptr<IExecutor> makeServiceExecutor(IEntry& entry) {
  return std::make_unique<DaemonExecutor>(entry);
}

}  // namespace Entries
}  // namespace Gengine
