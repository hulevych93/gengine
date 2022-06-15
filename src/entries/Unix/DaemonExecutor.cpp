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

DaemonExecutor* DaemonExecutor::m_instance = nullptr;

DaemonExecutor::DaemonExecutor(IEntry& entry)
    : SimpleExecutor(entry), m_stopEvent(Multithreading::ManualResetTag{}) {
  assert(!m_instance);
  m_instance = this;
}

void DaemonExecutor::SignalHandler(int signum) {
  if (m_instance) {
    m_instance->OnSignal(signum);
  }
}

void DaemonExecutor::OnSignal(int signum) {
  if (!m_signalWorker) {
    return;
  }

  switch (signum) {
    case SIGSEGV:
    case SIGABRT: {
      auto crashBuffer = new void*[200];
      auto crashAddressesNum = backtrace(crashBuffer, 200);

      auto handler = [signum, crashAddressesNum, crashBuffer]() mutable {
        char** strings;

        strings = backtrace_symbols(crashBuffer, crashAddressesNum);
        if (strings == NULL) {
          GLOG_INFO("backtrace_symbols failed, error == %d", errno);
        } else {
          std::string debugString = "\n";
          for (int j = 0; j < crashAddressesNum; j++) {
            debugString += boost::str(boost::format("%s\n") % strings[j]);
          }
          GLOG_INFO("%s", debugString.c_str());
          free(strings);
        }

        delete[] crashBuffer;

        // 2 sec additional sleep. seems dumper async or smth.
        usleep(2000 * 1000);

        signal(signum, SIG_DFL);
        raise(signum);
      };

      m_signalWorker->PostTask(std::move(handler));

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
      m_stopEvent.Set();
      _exit(1);
      break;
  }
}

bool DaemonExecutor::Execute(void* args) {
  pid_t pid, sid, parent;

  /* already a daemon */
  if (getppid() == 1) {
    // We're already a daemon. No actions performed
    return true;
  }

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

  m_signalWorker =
      std::make_unique<Multithreading::WorkerThread>("SignalHandlingThread");

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
    m_stopEvent.Wait(Multithreading::WaitInfinite);

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
