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
#include <driver_s5fh/S5FHPositionSettings.h>
#include <driver_s5fh/S5FHControllerFeedback.h>
#include <driver_s5fh/S5FHController.h>
#include <driver_s5fh/S5FHSerialPacket.h>
#include <boost/bind/bind.hpp>

using icl_comm::ArrayBuilder;
using namespace driver_s5fh;

void receivedPacketCallback(const S5FHSerialPacket& packet, unsigned int packet_count)
{
  std::cout << "Received new packet with number " << packet_count << std::endl;

  // Extract Channel
  u_int8_t channel = (packet.address >> 4 ) & 0x0F;
  // Prepare Data for conversion
  ArrayBuilder ab;
  ab.appendWithoutConversion(packet.data);

  std::cout << "channel = " << static_cast<int>(channel) << std::endl;

  if ((packet.address & 0x0F) == S5FH_SET_CONTROL_COMMAND) // || ((packet.address & 0x0F) == S5FH_SET_CONTROL_COMMAND)
  {
    S5FHControllerFeedback controller_feedback;

    ab >> controller_feedback;
    LOGGING_INFO_C(DriverS5FH, S5FHController, "Received a Control Feedback/Control Command packet for channel "<< channel << endl);

    std::cout << "Controller Feedback " << controller_feedback << endl;
  }
}

// testing serial interface of s5fh driver
int main(int argc, const char* argv[])
{
  icl_core::logging::initialize();

  std::string serial_device_name = "/dev/ttyUSB2";

  S5FHSerialInterface serial_com(serial_device_name, boost::bind(&receivedPacketCallback,_1,_2));

  while (true)
  { }
}

