// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Lars Pfotzer
 * \date    2014-02-15
 *
 */
//----------------------------------------------------------------------

#include <icl_comm/ByteOrderConversion.h>
#include <driver_s5fh/S5FHSerialInterface.h>
#include <driver_s5fh/S5FHControllerFeedback.h>
#include <driver_s5fh/S5FHController.h>
#include <driver_s5fh/S5FHSerialPacket.h>

using icl_comm::ArrayBuilder;
using namespace driver_s5fh;

using icl_comm::serial::Serial;
using icl_comm::serial::SerialFlags;

// testing serial interface of s5fh driver
int main(int argc, const char* argv[])
{
  icl_core::logging::initialize();

  std::string serial_device_name = "/dev/ttyUSB1";

  S5FHSerialInterface serial_com(NULL);
  serial_com.connect(serial_device_name);

  // build feedback serial packet for sending
  ArrayBuilder packet;
  S5FHCHANNEL channel = eS5FH_FINGER_SPREAD;
  S5FHSerialPacket test_serial_packet(40,S5FH_SET_CONTROL_COMMAND|static_cast<u_int8_t>(channel << 4));
  S5FHControllerFeedback test_controller_feedback(23,42);

  // serialize test controller feedback to paket
  packet << test_controller_feedback;
  test_serial_packet.index = 0;   //
  // Set the payload (converted array of position settings)
  test_serial_packet.data = packet.array;

  // send packet via serial port
  serial_com.sendPacket(test_serial_packet);

  serial_com.close();
}

