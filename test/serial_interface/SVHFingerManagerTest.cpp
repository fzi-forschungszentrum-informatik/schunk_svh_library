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

#include <schunk_svh_library/control/SVHFingerManager.h>
#include <schunk_svh_library/serial/SVHSerialPacket.h>
#include <thread>
#include <chrono>

using namespace driver_svh;

// testing serial interface of svh driver
int main(int argc, const char* argv[])
{
  icl_core::logging::initialize();

  std::string serial_device_name = "/dev/ttyUSB0";

  SVHFingerManager finger_manager;
  if (finger_manager.connect(serial_device_name))
  {
    SVHChannel test_channel = eSVH_ALL;

    std::cout << "connected" << std::endl;

    finger_manager.resetChannel(test_channel);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::cout << "Pinky is enabled: " << finger_manager.isEnabled(test_channel) << std::endl;
    std::cout << "Pinky is homed: " << finger_manager.isHomed(test_channel) << std::endl;

    std::cout << "Enabling Pinky: " << finger_manager.enableChannel(test_channel) << std::endl;


    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::cout << "Pinky is enabled: " << finger_manager.isEnabled(test_channel) << std::endl;
    std::cout << "Pinky is homed: " << finger_manager.isHomed(test_channel) << std::endl;


    std::this_thread::sleep_for(std::chrono::seconds(10));

    std::cout << "after sleep" << std::endl;

    finger_manager.disconnect();
  }

}
