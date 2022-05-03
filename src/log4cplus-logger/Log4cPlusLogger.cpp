#include <api/core/ILogger.h>

#include <log4cplus/configurator.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/layout.h>
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/ndc.h>
#include <log4cplus/win32debugappender.h>
#include <iostream>

#include <core/Encoding.h>
#include <filesystem/Filesystem.h>

#include <shared-services/SharedServiceExport.h>

#define LOG4CPLUS_STATIC
#define LOGS_DIR L"Logs"

#ifdef _WIN32
#define CONVERT_MESSAGE(MSG) MSG
#else
#define CONVERT_MESSAGE(MSG) toUtf8(MSG)
#endif

namespace Gengine {
using namespace Services;

static const std::string FileRolling = "File";
static const std::string Console = "Console";
static const std::string Win32Debug = "Win32Debug";
static const std::string PatternLayout =
    "%d{%d-%m-%y %H:%M:%S.%q} [%c{2}:%t] [%-5p] %m %n";

class Log4cPlusLogger : public ILogger {
 public:
  Log4cPlusLogger() : m_name() {
    log4cplus::initialize();  // init log4cplus

    auto root = log4cplus::Logger::getRoot();
#ifdef _WIN32
    {
      log4cplus::SharedAppenderPtr win32debugAppender(
          new log4cplus::Win32DebugAppender);
      win32debugAppender->setName(LOG4CPLUS_C_STR_TO_TSTRING(Win32Debug));
      win32debugAppender->setLayout(
          std::auto_ptr<log4cplus::Layout>(new log4cplus::PatternLayout(
              LOG4CPLUS_C_STR_TO_TSTRING(PatternLayout))));
      root.addAppender(win32debugAppender);
    }
#endif
  }

 protected:
  bool Init(const std::wstring& filename, bool useConsole) override {
    if (useConsole) {
      SetConsoleOutput(useConsole);
    } else {
      auto logFileName(filename);
      if (logFileName.empty()) {
        auto modulePath = Filesystem::GetExecutableFilePath();
        logFileName = Filesystem::GetFileName(modulePath);
      }

      auto moduleFolder = Filesystem::GetAppFolder();
      auto installFolder = Filesystem::CombinePath(moduleFolder, LOGS_DIR);

      Filesystem::CreateFolder(installFolder);
      logFileName = Filesystem::CombinePath(installFolder, logFileName);
      logFileName += L".log";

      SetFileRolling(toUtf8(logFileName));
    }

    return true;
  }

  void SetConsoleOutput(bool use_stdout) {
    auto root = log4cplus::Logger::getRoot();

    auto consoleAppender =
        root.getAppender(LOG4CPLUS_C_STR_TO_TSTRING(Console));
    if (consoleAppender) {
      if (!use_stdout) {
        root.removeAppender(LOG4CPLUS_C_STR_TO_TSTRING(Console));
      }
    } else {
      if (use_stdout) {
        log4cplus::SharedAppenderPtr consoleAppender(
            new log4cplus::ConsoleAppender());
        consoleAppender->setName(LOG4CPLUS_C_STR_TO_TSTRING(Console));
        consoleAppender->setLayout(
            std::auto_ptr<log4cplus::Layout>(new log4cplus::PatternLayout(
                LOG4CPLUS_C_STR_TO_TSTRING(PatternLayout))));
        root.addAppender(consoleAppender);
      }
    }
  }

  void SetFileRolling(const std::string& filename) {
    auto root = log4cplus::Logger::getRoot();

    if (!filename.empty()) {
      auto fileRollingAppender =
          root.getAppender(LOG4CPLUS_C_STR_TO_TSTRING(FileRolling));
      if (fileRollingAppender) {
      } else {
        log4cplus::SharedAppenderPtr fileAppender(
            new log4cplus::RollingFileAppender(
                LOG4CPLUS_C_STR_TO_TSTRING(filename), 5 * 1024 * 1024, 5));
        fileAppender->setName(LOG4CPLUS_C_STR_TO_TSTRING(FileRolling));
        fileAppender->setLayout(
            std::auto_ptr<log4cplus::Layout>(new log4cplus::PatternLayout(
                LOG4CPLUS_C_STR_TO_TSTRING(PatternLayout))));
        root.addAppender(fileAppender);
      }
    } else {
      root.removeAppender(LOG4CPLUS_C_STR_TO_TSTRING(FileRolling));
    }
  }

 public:
  bool LogTrace(const std::wstring& message) override {
    log4cplus::Logger current =
        log4cplus::Logger::getInstance(LOG4CPLUS_C_STR_TO_TSTRING(m_name));
    LOG4CPLUS_TRACE(current, CONVERT_MESSAGE(message));
    return true;
  }

  bool LogError(const std::wstring& message) override {
    auto current =
        log4cplus::Logger::getInstance(LOG4CPLUS_C_STR_TO_TSTRING(m_name));
    LOG4CPLUS_ERROR(current, CONVERT_MESSAGE(message));
    return true;
  }

  bool LogWarning(const std::wstring& message) override {
    auto current =
        log4cplus::Logger::getInstance(LOG4CPLUS_C_STR_TO_TSTRING(m_name));
    LOG4CPLUS_WARN(current, CONVERT_MESSAGE(message));
    return true;
  }

  bool LogInfo(const std::wstring& message) override {
    auto current =
        log4cplus::Logger::getInstance(LOG4CPLUS_C_STR_TO_TSTRING(m_name));
    LOG4CPLUS_INFO(current, CONVERT_MESSAGE(message));
    return true;
  }

  bool LogDebug(const std::wstring& message) override {
    auto current =
        log4cplus::Logger::getInstance(LOG4CPLUS_C_STR_TO_TSTRING(m_name));
    LOG4CPLUS_DEBUG(current, CONVERT_MESSAGE(message));
    return true;
  }

 private:
  const std::string m_name;
};

EXPORT_GLOBAL_SHARED_SERVICE(Log4cPlusLogger)

}  // namespace Gengine
