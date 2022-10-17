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
 * \date    2014-02-10
 * \date    2014-07-16
 *
 * This file contains the SVHSerialPacket data structure that is used to
 * send and receive everything from an to the actual hardware.
 * The serial packet is used to wrap up the payload data for convenient handling.
 * By wrapping everything in the same packet structure it can be handled quite neatly
 */
//----------------------------------------------------------------------
#include <schunk_svh_library/serial/SVHSerialPacket.h>

namespace driver_svh {

driver_svh::ArrayBuilder& operator<<(driver_svh::ArrayBuilder& ab, const SVHSerialPacket& data)
{
  ab << data.index << data.address << static_cast<uint16_t>(data.data.size()) << data.data;
  return ab;
}

driver_svh::ArrayBuilder& operator>>(driver_svh::ArrayBuilder& ab, SVHSerialPacket& data)
{
  // Disregard the size when deserializing as we get that anyway
  uint16_t size;
  ab >> data.index >> data.address >> size >> data.data;
  return ab;
}

std::ostream& operator<<(std::ostream& o, const SVHSerialPacket& sp)
{
  o << "index: " << static_cast<int>(sp.index) << " address: "
    << "0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(sp.address)
    << " Data: ";
  for (size_t i = 0; i < sp.data.size(); i++)
  {
    o << "0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(sp.data[i])
      << " ";
  }
  // Reset Output stream to decimal output .. otherwise it may confuse people and the stream
  // operators have the tendency to hang on to these hints
  std::cout << std::dec;
  return o;
}

} // namespace driver_svh
