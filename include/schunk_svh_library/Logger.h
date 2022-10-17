////////////////////////////////////////////////////////////////////////////////
//
// © Copyright 2022 SCHUNK Mobile Greifsysteme GmbH, Lauffen/Neckar Germany
// © Copyright 2022 FZI Forschungszentrum Informatik, Karlsruhe, Germany
//
// This file is part of the Schunk SVH Library.
//
// The Schunk SVH Library is free software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
//
// The Schunk SVH Library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
// Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// the Schunk SVH Library. If not, see <https://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////

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
