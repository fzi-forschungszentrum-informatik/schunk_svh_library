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
 * \author Kay-Ulrich Scholl
 * \date 05.12.00
 * \author  Lars Pfotzer <pfotzer@fzi.de>
 * \date    2013-03-08
 *
 * \brief Contains a class that enables access to serial devices
 *
 * stole lots of the code from mcal_hardware/tSerial.*
 *
 */
//----------------------------------------------------------------------
#ifndef DRIVER_SVH_SERIAL_SERIAL_H_INCLUDED
#define DRIVER_SVH_SERIAL_SERIAL_H_INCLUDED

#include <string>
#include <sys/types.h>

#include "schunk_svh_library/ImportExport.h"
#include "schunk_svh_library/serial/SerialFlags.h"

#ifdef _SYSTEM_WIN32_
typedef unsigned int speed_t;
typedef int ssize_t;
#endif

#ifdef _SYSTEM_POSIX_
#  include <unistd.h>
#  ifdef _SYSTEM_DARWIN_
#    include <termios.h>
#  else
#    include <termio.h>
#  endif
#endif

namespace driver_svh {
namespace serial {

//! Enables acces to serial devices
/*!
  Open a serial device, change baudrates, read from and write to the device.
  Status-information after calling the functions get be
 */
class DRIVER_SVH_IMPORT_EXPORT Serial
{
public:
  /*!
    Opens the device (if possible) with the given flags.
    \param flags can be values of the following values ORed together
    - CS5, CS6, CS7, or CS8  databits
    - CSTOPB set two stop bits, rather than one.
    - CREAD  enable receiver.
    - PARENB enable  parity  generation  on  output  and  parity
              checking for input.
    - PARODD parity for input and output is odd.
    - HUPCL  lower modem control lines after last process closes
              the device (hang up).
    - CLOCAL ignore modem control lines
    - B50, B75, B110, B134, B150, B200, B300, B600, B1200,
     B1800, B2400, B4800, B9600, B19200, B38400, B57600,
     B115200, B230400, B921600      Baudrates
    - CRTSCTS  flow control.
  */
  Serial(const char* dev_name, const SerialFlags& flags);

  /*!
   * This is an overloaded constructor, provided for convenience.
   * \a buad_rate overwrites the baud rate set in \a flags.
   */
  Serial(const char* dev_name, SerialFlags::BaudRate baud_rate, const SerialFlags& flags);

  /*!
    Restore old serial settings and close device
   */
  ~Serial();
  /*!
    speed is one of the followind values :
    - B50, B75, B110, B134, B150, B200, B300, B600, B1200,
    B1800, B2400, B4800, B9600, B19200, B38400, B57600,
    B115200, B230400, B921600
    Returns 0 on success.
    Returns -status on failure.
   */
  int changeBaudrate(SerialFlags::BaudRate speed);

  /*!
   * Clears the serial port's receive buffer.
   */
  int clearReceiveBuffer();

  /*!
   * Clears the serial port's send buffer.
   */
  int clearSendBuffer();

  /*!
    Opens the serial interface with the given \a flags
   */
  bool open(const SerialFlags& flags)
  {
    m_serial_flags = flags;
    return open();
  }

  /*!
    Opens the serial interface.
   */
  bool open();

  /*!
    Returns \c true if the serial interface is opened.
    \c false otherwhise.
   */
  bool isOpen() const;

  /*!
    Close the serial interface.
   */
  void close();


  /*!
    Write data to serial out.
   */
  ssize_t write(const void* data, ssize_t size);
  /*!
    Read data from device. This function waits until \param time us passed or
    the respected number of bytes are received via serial line.
    if (\param return_on_less_data is true (default value), the number of bytes
    that have been receives are returned and the data is stored in \param data.
    If the parameter is false, data is only read from serial line, if at least
    \param size bytes are available.
   */
  ssize_t read(void* data, ssize_t size, unsigned long time = 100, bool return_on_less_data = true);
  /*!
    All routines return a negavtiv number on error. Then the global errno is
    stored into a private variable. Use this funtion to ask for this value.
    Especially the constructor cannot return an error-value. Check the
    succesful opening of the device by calling this function. It returns 0, if
    no error occured.
   */
  int status() const { return m_status; }

  std::string statusText() const;

  const char* deviceName() const { return m_dev_name; }

  /*!
    Return the file descriptor of the serial class
  */
  int fileDescriptor()
  {
#ifdef _SYSTEM_POSIX_
    return m_file_descr;
#else
    return 0;
#endif
  }

private:
  // Forbid copying.
  Serial(const Serial&);
  Serial& operator=(const Serial&);

  void dumpData(void* data, size_t length);

#ifdef _SYSTEM_WIN32_
  HANDLE m_com;
  unsigned char m_read_buffer[0x4000];
  ssize_t m_read_buffer_fill;
#endif

#ifdef _SYSTEM_POSIX_
  int m_file_descr;
  termios m_io_set_old;
#endif

  char* m_dev_name;
  SerialFlags m_serial_flags;
  int m_status;
};

} // namespace serial
} // namespace driver_svh

#endif
