#include <entries/Unix/DaemonExecutor.h>
#include <entries/diagnostic/Unix/LinuxCallstackDumper.h>

#include <errno.h>
#include <signal.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

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
  if (m_terminated.load()) {
    return;
  }

  switch (signum) {
    case SIGSEGV:
    case SIGABRT: {
      m_terminated.store(true);
      Diagnostic::LinuxCallstackDumper::Backtrace();
      _exit(1);
    } break;
    case SIGALRM:
    case SIGUSR1:
    case SIGCHLD:
      _exit(1);
      break;
    case SIGTERM:
      m_terminated.store(true);
      m_stopEvent.Set();
      break;
  }
}

bool DaemonExecutor::Execute(void* args) {
  if (getppid() == 1) {
    return true;
  }

  signal(SIGCHLD, SignalHandler);
  signal(SIGUSR1, SignalHandler);
  signal(SIGALRM, SignalHandler);
  signal(SIGTERM, SignalHandler);
  signal(SIGSEGV, SignalHandler);
  signal(SIGABRT, SignalHandler);

  auto pid = fork();
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

  /* At this point we are executing as the child process */
  auto parent = getppid();

  /* Cancel certain signals */
  signal(SIGCHLD, SIG_DFL); /* A child process dies */
  signal(SIGTSTP, SIG_IGN); /* Various TTY signals */
  signal(SIGTTOU, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGHUP, SIG_IGN); /* Ignore hangup signal */

  /* Change the file mode mask */
  umask(0);

  /* Create a new SID for the child process */
  auto sid = setsid();
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

  auto& entry = GetEntry();
  if (entry.Initialize()) {
    entry.Execute(args);
    m_stopEvent.Wait(Multithreading::WaitInfinite);
    entry.Exit(&m_code);
    entry.Finalize();
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
