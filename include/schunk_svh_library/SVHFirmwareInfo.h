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
 * \author  Georg Heppner
 * \date    2014-02-03
 * \date    2014-07-16
 *
 * This file contains the SVHFirmwareInfo data structure that is used to
 * receive information about the current firmware settings
 */
//----------------------------------------------------------------------
#ifndef SVHFIRMWAREINFO_H
#define SVHFIRMWAREINFO_H

#include <iostream>
#include <string>
#include <vector>

#include <schunk_svh_library/serial/ByteOrderConversion.h>

namespace driver_svh {

/*!
 * \brief The SVHFirmwareInfo  holds the data of a firmware response from the hardware
 */
struct SVHFirmwareInfo
{
  //! 4 bytes identifier
  std::string svh;
  //! Major version number
  uint16_t version_major;
  //! Minor version number
  uint16_t version_minor;
  //! 48 bytes! of text (free)
  std::string text;

  //! Compares two SVHFirmware objects.
  bool operator==(const SVHFirmwareInfo& other) const
  {
    return (version_major == other.version_major && version_minor == other.version_minor);
  }
};

//! overload stream operator to easily serialize firmware data
inline driver_svh::ArrayBuilder& operator<<(driver_svh::ArrayBuilder& ab, SVHFirmwareInfo& data)
{
  // Stream operator can not handle arrays (due to missing size information) to make things easy we
  // just copy the data around. Feel free to do something else Todo: The conversion in this
  // direction is not properly working, as the readout is working fine so far this is a fix for
  // later. It is probably something that has to do with the data types and how that is interpreted
  // by the arrayBuilder
  std::vector<int8_t> text(48);
  std::vector<int8_t> svh(4);
  svh.insert(svh.begin(), data.svh.begin(), data.svh.end());
  text.insert(text.begin(), data.text.begin(), data.text.end());

  ab << svh << data.version_major << data.version_minor << text;
  return ab;
}


//! overload stream operator to easily serialize firmware data
inline driver_svh::ArrayBuilder& operator>>(driver_svh::ArrayBuilder& ab, SVHFirmwareInfo& data)
{
  // Stream operator can not handle arrays (due to missing size information) to make things easy we
  // just copy the data around. Feel free to do something else
  std::vector<uint8_t> text(48);
  std::vector<uint8_t> svh(4);

  ab >> svh >> data.version_major >> data.version_minor >> text;


  data.text = std::string(text.begin(), text.end());
  data.svh  = std::string(svh.begin(), svh.end());
  //  std::copy (text.begin(),text.end(),data.text);
  //  std::copy (svh.begin(),svh.end(),data.svh);

  return ab;
}

//! Output Stream operator for easy output of the firmware information
inline std::ostream& operator<<(std::ostream& o, const SVHFirmwareInfo& fw)
{
  o << fw.svh.c_str() << " " << fw.version_major << "." << fw.version_minor << " : "
    << fw.text.c_str() << std::endl;
  return o;
}


} // namespace driver_svh


#endif // SVHFIRMWAREINFO_H
