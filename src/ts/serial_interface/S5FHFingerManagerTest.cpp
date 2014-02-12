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

#include <driver_s5fh/S5FHFingerManager.h>
#include <driver_s5fh/S5FHSerialPacket.h>

using namespace driver_s5fh;

// testing serial interface of s5fh driver
int main(int argc, const char* argv[])
{
  icl_core::logging::initialize();

  std::string serial_device_name = "/dev/ttyUSB1";

  S5FHFingerManager finger_manager;
  if (finger_manager.connect(serial_device_name))
  {
    S5FHCHANNEL test_channel = eS5FH_PINKY;

    icl_core::os::sleep(2);

    finger_manager.resetChannel(test_channel);

    icl_core::os::sleep(2);

    std::cout << "Pinky is enabled: " << finger_manager.isEnabled(test_channel) << std::endl;
    std::cout << "Pinky is homed: " << finger_manager.isHomed(test_channel) << std::endl;

    icl_core::os::sleep(2);

    finger_manager.disconnect();
  }

}
