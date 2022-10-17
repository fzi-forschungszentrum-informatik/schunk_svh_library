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
 * \date    2014-02-15
 *
 */
//----------------------------------------------------------------------

#include <schunk_svh_library/control/SVHPositionSettings.h>
#include <schunk_svh_library/serial/ByteOrderConversion.h>
#include <schunk_svh_library/serial/SVHSerialInterface.h>
#include <schunk_svh_library/serial/Serial.h>

using driver_svh::ArrayBuilder;
using namespace driver_svh;

using driver_svh::serial::Serial;
using driver_svh::serial::SerialFlags;

// testing serial interface of svh driver
int main(int argc, const char* argv[])
{
  std::string serial_device_name = "/dev/ttyUSB0";

  SVHSerialInterface serial_com(NULL);
  serial_com.connect(serial_device_name);

  // build serial packet for sending
  ArrayBuilder payload(40);
  SVHSerialPacket test_serial_packet;
  SVHPositionSettings test_pos_settings(0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.1);

  // Get the Position settings as payload
  payload << test_pos_settings;
  // Generate the header information
  test_serial_packet.address = 5; // Set Position settings
  test_serial_packet.index   = 0; //
  // Set the payload (converted array of position settings)
  test_serial_packet.data = payload.array;

  // send packet via serial port
  serial_com.sendPacket(test_serial_packet);

  serial_com.close();
}
