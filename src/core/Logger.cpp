#include <core/Logger.h>

#include <core/Encoding.h>
#include <filesystem/Filesystem.h>
#include <shared-services/SharedServiceImport.h>

namespace Gengine {
using namespace Services;
using namespace SharedServices;

std::shared_ptr<ILoggerProxy> GetLoggerProxy() {
  static std::shared_ptr<ILoggerProxy> LoggerProxy;
  if (!LoggerProxy) {
    SharedConnection connection;
    connection.path = "logger-proxy";
    connection.symbol = "LoggerProxy_service";
    LoggerProxy = import_symbol<ILoggerProxy>(connection);
  }
  assert(LoggerProxy);
  return LoggerProxy;
}

Logger::Logger() = default;
Logger::~Logger() = default;

void Logger::Init(const std::string& filename, bool useConsole) {
  try {
    GetLoggerProxy()->Init(filename, useConsole);
  } catch (std::exception&) {
  }
}

void Logger::Deinit() {
  try {
    GetLoggerProxy()->Deinit();
  } catch (std::exception&) {
  }
}

void Logger::Log(const char* function_name,
                 std::size_t line,
                 type_t type,
                 level_t level,
                 const char* fmt,
                 va_list args) const {
  try {
    if (GetLoggerProxy()->IsAvailable(type)) {
      static const int DefaultBufferSize = 1024;
      int bufferSize = DefaultBufferSize;
      std::unique_ptr<char[]> buffer;
      boost::format format;

      bool packed = false;
      while (!packed) {
        buffer = std::make_unique<char[]>(bufferSize);
        buffer[bufferSize - 1] = 0;

        auto coded = vsnprintf(buffer.get(), bufferSize, fmt, args);
        if (coded >= 0) {
          buffer[bufferSize - 1] = 0;
          packed = true;
        } else {
          int iErrno = errno;
          if (iErrno == EILSEQ ||  // character cannot be converted to ASCII
                                   // (May be on MS Windows systems)
              iErrno == EINVAL)    // invalid format string
          {                        // always ensure buffer is zero terminated
            buffer[bufferSize - 1] = 0;
            packed = true;
          } else {  // seems, not enough buffer space; increasebuffer and try
                    // again
            buffer.reset();
            bufferSize += DefaultBufferSize;
          }
        }
      }

      format = boost::format(buffer.get());
      format.exceptions(boost::io::no_error_bits);
      Log(function_name, line, type, level, format);
    }
  } catch (std::exception&) {
  }
}

void Logger::Log(const char* function_name,
                 std::size_t line,
                 type_t type,
                 level_t level,
                 boost::format& format) const {
  try {
    if (GetLoggerProxy()->IsAvailable(type)) {
      std::string prefix;
      if (function_name && line > 0) {
        prefix =
            boost::str(boost::format("[%1%:%2%]: ") % function_name % line);
      }
      auto data = utf8toWchar(prefix + format.str());
      GetLoggerProxy()->Log(type, level, data);
    }
  } catch (std::exception&) {
  }
}

std::string Logger::Process(const std::wstring& value) {
  return toUtf8(value);
}

std::string Logger::Process(const wchar_t* value) {
  return Process(std::wstring(value));
}
}  // namespace Gengine
