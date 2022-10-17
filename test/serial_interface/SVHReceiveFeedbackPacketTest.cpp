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

#include <boost/bind/bind.hpp>
#include <schunk_svh_library/control/SVHController.h>
#include <schunk_svh_library/control/SVHControllerFeedback.h>
#include <schunk_svh_library/control/SVHPositionSettings.h>
#include <schunk_svh_library/serial/ByteOrderConversion.h>
#include <schunk_svh_library/serial/SVHSerialPacket.h>

using driver_svh::ArrayBuilder;
using namespace driver_svh;
#if BOOST_VERSION >= 106000 // Moved to namespace in boost 1.60
using namespace boost::placeholders;
#endif

void receivedPacketCallback(const SVHSerialPacket& packet, unsigned int packet_count)
{
  std::cout << "Received new packet with number " << packet_count << std::endl;

  // Extract Channel
  uint8_t channel = (packet.address >> 4) & 0x0F;
  // Prepare Data for conversion
  ArrayBuilder ab;
  ab.appendWithoutConversion(packet.data);

  std::cout << "channel = " << static_cast<int>(channel) << std::endl;

  if ((packet.address & 0x0F) ==
      SVH_SET_CONTROL_COMMAND) // || ((packet.address & 0x0F) == SVH_SET_CONTROL_COMMAND)
  {
    SVHControllerFeedback controller_feedback;

    ab >> controller_feedback;
    SVH_LOG_INFO_STREAM("SVHController",
                        "Received a Control Feedback/Control Command packet for channel "
                          << channel);

    std::cout << "Controller Feedback " << controller_feedback << std::endl;
  }
}

// testing serial interface of svh driver
int main(int argc, const char* argv[])
{
  icl_core::logging::initialize();

  std::string serial_device_name = "/dev/ttyUSB0";

  SVHSerialInterface serial_com(boost::bind(&receivedPacketCallback, _1, _2));
  serial_com.connect(serial_device_name);

  while (true)
  {
  }

  serial_com.close();
}
