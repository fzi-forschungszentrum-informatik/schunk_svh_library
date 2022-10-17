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
 * \author  Lars Pfotzer
 * \date    2014-01-30
 *
 * Defines used for windows DLL support.
 */
//----------------------------------------------------------------------
#ifndef DRIVER_SVH_IMPORT_EXPORT_H_INCLUDED
#define DRIVER_SVH_IMPORT_EXPORT_H_INCLUDED

#if defined(_SYSTEM_WIN32_) && !defined(_IC_STATIC_)
#  pragma warning(disable : 4251)

#  if defined DRIVER_SVH_EXPORT_SYMBOLS
#    define DRIVER_SVH_IMPORT_EXPORT __declspec(dllexport)
#  else
#    define DRIVER_SVH_IMPORT_EXPORT __declspec(dllimport)
#  endif

#elif defined(__GNUC__) && (__GNUC__ > 3) && !defined(_IC_STATIC_)

#  define DRIVER_SVH_IMPORT_EXPORT __attribute__((visibility("default")))

#else

#  define DRIVER_SVH_IMPORT_EXPORT

#endif

#endif
