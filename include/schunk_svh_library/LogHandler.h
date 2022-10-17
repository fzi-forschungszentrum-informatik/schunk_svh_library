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
