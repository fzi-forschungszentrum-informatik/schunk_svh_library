// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Felix Exner exner@fzi.de
 * \date    2022-01-13
 *
 */
//----------------------------------------------------------------------
#pragma once

#include <string>

#include <schunk_svh_library/LogLevel.h>

namespace driver_svh {

class LogHandler
{
public:
  LogHandler()          = default;
  virtual ~LogHandler() = default;

  virtual void log(const std::string& file,
                   const int line,
                   const std::string& name,
                   LogLevel level,
                   const std::string& msg) = 0;
};

class ShellLogHandler : public LogHandler
{
public:
  ShellLogHandler()                   = default;
  virtual ~ShellLogHandler() override = default;
  virtual void log(const std::string& file,
                   const int line,
                   const std::string& name,
                   LogLevel level,
                   const std::string& msg) override;

private:
};

} // namespace driver_svh
