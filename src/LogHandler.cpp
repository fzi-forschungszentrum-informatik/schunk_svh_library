// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Felix Exner mauch@fzi.de
 * \date    2022-01-13
 *
 */
//----------------------------------------------------------------------

#include <iostream>

#include <schunk_svh_library/LogHandler.h>
#include <schunk_svh_library/LogLevel.h>

namespace driver_svh {
void ShellLogHandler::log(const std::string& file,
                          const int line,
                          const std::string& name,
                          LogLevel level,
                          const std::string& msg)
{
  std::cout << logLevelToString(level) << " " << name << " (" << file << ", line " << line
            << "): " << msg << std::endl;
}
} // namespace driver_svh
