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

#include <schunk_svh_library/serial/ByteOrderConversion.h>
#include <schunk_svh_library/serial/Serial.h>

using driver_svh::ArrayBuilder;
using driver_svh::serial::Serial;
using driver_svh::serial::SerialFlags;

// testing serial interface of svh driver
int main(int argc, const char* argv[])
{
  std::string serial_device_name = "/dev/ttyUSB1";

  Serial* serial_device =
    new Serial(serial_device_name.c_str(), SerialFlags(SerialFlags::BR_921600, SerialFlags::DB_8));
  serial_device->open();

  uint8_t data = 0;
  while (true)
  {
    if (serial_device->read(&data, sizeof(uint8_t)))
    {
      std::cout << "0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(data)
                << " " << std::flush;
    }
    else
    {
      std::cout << "." << std::flush;
    }
  }

  serial_device->close();
}
