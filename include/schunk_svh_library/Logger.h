// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-
//----------------------------------------------------------------------
/*!\file
 *
 * \author  Felix Exner
 * \date    2022-01-13
 *
 */
//----------------------------------------------------------------------
#pragma once

#include <memory>
#include <sstream>

#include <schunk_svh_library/LogHandler.h>
#include <schunk_svh_library/LogLevel.h>

#define SVH_LOG_DEBUG_STREAM(NAME, M)                                                              \
  do                                                                                               \
  {                                                                                                \
    std::stringstream ss;                                                                          \
    ss << M;                                                                                       \
    Logger::log(__FILE__, __LINE__, NAME, driver_svh::LogLevel::DEBUG, ss.str());                  \
  } while (false)
#define SVH_LOG_INFO_STREAM(NAME, M)                                                               \
  do                                                                                               \
  {                                                                                                \
    std::stringstream ss;                                                                          \
    ss << M;                                                                                       \
    Logger::log(__FILE__, __LINE__, NAME, driver_svh::LogLevel::INFO, ss.str());                   \
  } while (false)
#define SVH_LOG_WARN_STREAM(NAME, M)                                                               \
  do                                                                                               \
  {                                                                                                \
    std::stringstream ss;                                                                          \
    ss << M;                                                                                       \
    Logger::log(__FILE__, __LINE__, NAME, driver_svh::LogLevel::WARN, ss.str());                   \
  } while (false)
#define SVH_LOG_ERROR_STREAM(NAME, M)                                                              \
  do                                                                                               \
  {                                                                                                \
    std::stringstream ss;                                                                          \
    ss << M;                                                                                       \
    Logger::log(__FILE__, __LINE__, NAME, driver_svh::LogLevel::ERROR, ss.str());                  \
  } while (false)
#define SVH_LOG_FATAL_STREAM(NAME, M)                                                              \
  do                                                                                               \
  {                                                                                                \
    std::stringstream ss;                                                                          \
    ss << M;                                                                                       \
    Logger::log(__FILE__, __LINE__, NAME, driver_svh::LogLevel::FATAL, ss.str());                  \
  } while (false)

namespace driver_svh {

/*!
 * \brief Small singleton-based logging class with an exchangeable log handler
 */
class Logger
{
public:
  static Logger& getInstance()
  {
    static Logger instance;
    return instance;
  }
  Logger(const Logger&) = delete;
  void operator=(Logger const&) = delete;
  virtual ~Logger()             = default;

  static void setLogHandler(std::unique_ptr<LogHandler> log_handler)
  {
    Logger& logger       = getInstance();
    logger.m_log_handler = std::move(log_handler);
  }

  static void setLogLevel(const LogLevel& log_level)
  {
    Logger& logger     = getInstance();
    logger.m_log_level = log_level;
  }

  static void log(const std::string& file,
                  const int line,
                  const std::string& name,
                  const LogLevel level,
                  const std::string& msg...)
  {
    Logger& logger = getInstance();
    if (level >= logger.m_log_level)
    {
      logger.m_log_handler->log(file, line, name, level, msg);
    }
  }


private:
  Logger()
    : m_log_handler(new ShellLogHandler()){};

  LogLevel m_log_level = LogLevel::WARN;
  std::unique_ptr<LogHandler> m_log_handler;
};

} // namespace driver_svh
