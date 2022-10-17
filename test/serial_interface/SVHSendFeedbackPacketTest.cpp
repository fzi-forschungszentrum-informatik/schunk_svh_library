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

#include <chrono>
#include <schunk_svh_library/control/SVHController.h>
#include <schunk_svh_library/control/SVHControllerFeedback.h>
#include <schunk_svh_library/serial/ByteOrderConversion.h>
#include <schunk_svh_library/serial/SVHSerialInterface.h>
#include <schunk_svh_library/serial/SVHSerialPacket.h>
#include <thread>

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

  // build feedback serial packet for sending
  ArrayBuilder packet;
  SVHChannel channel = SVH_PINKY;
  SVHSerialPacket test_serial_packet(40,
                                     SVH_SET_CONTROL_COMMAND | static_cast<uint8_t>(channel << 4));
  SVHControllerFeedback test_controller_feedback(0, 140);

  // serialize test controller feedback to paket
  packet << test_controller_feedback;
  test_serial_packet.index = 0; //
  // Set the payload (converted array of position settings)
  test_serial_packet.data = packet.array;

  // send packet via serial port
  serial_com.sendPacket(test_serial_packet);

  std::this_thread::sleep_for(std::chrono::seconds(5));

  test_controller_feedback.position = -8000;

  // serialize test controller feedback to paket
  packet.reset(0);
  packet << test_controller_feedback;
  test_serial_packet.index = 0; //
  // Set the payload (converted array of position settings)
  test_serial_packet.data = packet.array;

  // send packet via serial port
  serial_com.sendPacket(test_serial_packet);

  serial_com.close();
}
