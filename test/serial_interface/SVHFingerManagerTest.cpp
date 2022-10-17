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
#include <schunk_svh_library/control/SVHFingerManager.h>
#include <schunk_svh_library/serial/SVHSerialPacket.h>
#include <thread>

using namespace driver_svh;

// testing serial interface of svh driver
int main(int argc, const char* argv[])
{
  std::string serial_device_name = "/dev/ttyUSB0";

  SVHFingerManager finger_manager;
  if (finger_manager.connect(serial_device_name))
  {
    SVHChannel test_channel = SVH_ALL;

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
