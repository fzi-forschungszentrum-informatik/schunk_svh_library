// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// This file is part of the SCHUNK SVH Driver suite.
//
// This program is free software licensed under the LGPL
// (GNU LESSER GENERAL PUBLIC LICENSE Version 3).
// You can find a copy of this license in LICENSE folder in the top
// directory of the source code.
//
// © Copyright 2014 SCHUNK Mobile Greifsysteme GmbH, Lauffen/Neckar Germany
// © Copyright 2014 FZI Forschungszentrum Informatik, Karlsruhe, Germany
//
// -- END LICENSE BLOCK ------------------------------------------------

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
