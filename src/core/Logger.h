#pragma once

#include <boost/format.hpp>
#include <string>

namespace Gengine {

enum class type_t : std::uint8_t { Internal = 0, Regular };

enum class level_t : std::uint8_t { Trace = 0, Error, Warning, Info, Debug };

class ILoggerProxy {
 public:
  virtual ~ILoggerProxy() = default;
  virtual void Init(const std::string& filename, bool useConsole) = 0;
  virtual void Deinit() = 0;
  virtual void Log(type_t type, level_t level, const std::wstring& data) = 0;
  virtual bool IsAvailable(type_t type) const = 0;
};

class Logger {
 public:
  Logger(const Logger&) = delete;
  Logger(Logger&&) = delete;
  ~Logger();

  Logger& operator=(const Logger&) = delete;
  Logger& operator=(Logger&&) = delete;

  void Init(const std::string& filename = "", bool useConsole = false);
  void Deinit();

  static Logger& GetInstance() {
    static Logger instance;
    return instance;
  }

  template <typename... Args>
  void operator()(const char* function_name,
                  std::size_t line,
                  type_t type,
                  level_t level,
                  const std::string& fmt,
                  const Args&... args) const {
    boost::format format(fmt);
    format.exceptions(boost::io::no_error_bits);
    Log(function_name, line, type, level, format, args...);
  }

 private:
  Logger();

  void Log(const char* function_name,
           std::size_t line,
           type_t type,
           level_t level,
           boost::format& format) const;
  void Log(const char* function_name,
           std::size_t line,
           type_t type,
           level_t level,
           const char* fmt,
           va_list args) const;

  template <typename Arg, typename... Args>
  void Log(const char* function_name,
           std::size_t line,
           type_t type,
           level_t level,
           boost::format& format,
           Arg arg,
           const Args&... args) const {
    format % Process(arg);
    Log(function_name, line, type, level, format, args...);
  }

  template <typename... Args>
  void Log(const char* function_name,
           std::size_t line,
           type_t type,
           level_t level,
           const std::string& fmt,
           const Args&... args) const {
    boost::format format(fmt);
    format.exceptions(boost::io::no_error_bits);
    Log(function_name, line, type, level, format, args...);
  }

  template <typename Type>
  Type Process(const Type& value) const {
    return value;
  }

  static std::string Process(const std::wstring& value);
  static std::string Process(const wchar_t* value);
};

}  // namespace Gengine

#define GLOGGER Logger::GetInstance()
#define GLOG_INIT GLOGGER.Init
#define GLOG_DEINIT GLOGGER.Deinit()
#define LOG(type, level, str, ...) \
  GLOGGER(__FUNCTION__, __LINE__, type, level, str, ##__VA_ARGS__)

#if defined(_DEBUG)
#define GLOG_DEBUG(str, ...)                                             \
  GLOGGER(__FUNCTION__, __LINE__, type_t::Internal, level_t::Debug, str, \
          ##__VA_ARGS__)
#define GLOG_DEBUG_INTERNAL(str, ...)                                    \
  GLOGGER(__FUNCTION__, __LINE__, type_t::Internal, level_t::Debug, str, \
          ##__VA_ARGS__)
#else
#define GLOG_DEBUG(str, ...)                                            \
  GLOGGER(__FUNCTION__, __LINE__, type_t::Regular, level_t::Debug, str, \
          ##__VA_ARGS__)
#define GLOG_DEBUG_INTERNAL(str, ...)
#endif

#define GLOG_INFO(str, ...) \
  LOG(type_t::Regular, level_t::Info, str, ##__VA_ARGS__)
#define GLOG_WARNING(str, ...) \
  LOG(type_t::Regular, level_t::Warning, str, ##__VA_ARGS__)
#define GLOG_ERROR(str, ...) \
  LOG(type_t::Regular, level_t::Error, str, ##__VA_ARGS__)

#define GLOG_INFO_INTERNAL(str, ...) \
  LOG(type_t::Internal, level_t::Info, str, ##__VA_ARGS__)
#define GLOG_WARNING_INTERNAL(str, ...) \
  LOG(type_t::Internal, level_t::Warning, str, ##__VA_ARGS__)
#define GLOG_ERROR_INTERNAL(str, ...) \
  LOG(type_t::Internal, level_t::Error, str, ##__VA_ARGS__)
