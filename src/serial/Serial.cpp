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
 *
 * \author Kay-Ulrich Scholl
 * \date 05.12.00
 *
 *
 * \author  Lars Pfotzer <pfotzer@fzi.de>
 * \date    2013-03-08
 *
 *
 */
//----------------------------------------------------------------------

// Not functional with Mac OS X (UH)

#include "schunk_svh_library/serial/Serial.h"
#include "schunk_svh_library/Logger.h"

#include <algorithm>
#include <cassert>

#include <ratio>

#ifdef _SYSTEM_LINUX_
#  include <errno.h>
#  include <fcntl.h>
#  include <stdio.h>
#  include <string.h>
#  include <sys/time.h>
#endif

#include <chrono>
//#include "mcal_math/OwnMath.h"

#ifdef _SYSTEM_WIN32_
#  include <math.h>
#  include <stdio.h>
#endif

#undef SERIAL_DUMP_DATA
//#define SERIAL_DUMP_DATA

namespace driver_svh {
namespace serial {

Serial::Serial(const char* dev_name, const SerialFlags& flags)
  : m_dev_name(strdup(dev_name))
  , m_serial_flags(flags)
{
#ifdef _SYSTEM_WIN32_
  m_com = INVALID_HANDLE_VALUE;
#else
  m_file_descr = -1;
#endif

  open();
}

Serial::Serial(const char* dev_name, SerialFlags::BaudRate baud_rate, const SerialFlags& flags)
  : m_dev_name(strdup(dev_name))
  , m_serial_flags(flags)
{
#ifdef _SYSTEM_WIN32_
  m_com = INVALID_HANDLE_VALUE;
#else
  m_file_descr = -1;
#endif

  m_serial_flags.setBaudRate(baud_rate);
  open();
}

bool Serial::open()
{
  close();

#if defined _SYSTEM_LINUX_
  // Attention! The following code will be executed,
  // if we are not a lxrt-task or no lxrt interface is available:
  {
    termios io_set_new;

    // open device
    if ((m_file_descr = ::open(m_dev_name, O_RDWR | O_NONBLOCK)) < 0)
    {
      m_status = -errno;
      SVH_LOG_DEBUG_STREAM("Serial",
                           "Cannot open serial device '" << m_dev_name << "'. Status (" << m_status
                                                         << ":" << strerror(-m_status));
      // DEBUGMSG(DD_SYSTEM, DL_DEBUG, "Cannot open serial device '%s'. Status (%i:%s)\n",
      // m_dev_name, m_status, strerror(-m_status));
      return false;
    }
    else
      m_status = 0;

    // get device-settings
    if (tcgetattr(m_file_descr, &m_io_set_old) < 0)
    {
      m_status = -errno;
      SVH_LOG_DEBUG_STREAM("Serial",
                           "Cannot get serial device m_status '" << m_dev_name << "'. Status ("
                                                                 << m_status << ":"
                                                                 << strerror(-m_status));
      // DEBUGMSG(DD_SYSTEM, DL_DEBUG, "Cannot get serial device m_status '%s'. Status (%i:%s)\n",
      // m_dev_name, m_status, strerror(-m_status));
      return false;
    }
    else
      m_status = 0;

    // copy settings from old settings
    io_set_new = m_io_set_old;

    // declare new settings
    io_set_new.c_cflag     = m_serial_flags.cFlags();
    io_set_new.c_oflag     = 0;
    io_set_new.c_iflag     = IGNPAR;
    io_set_new.c_lflag     = 0;
    io_set_new.c_cc[VMIN]  = 1;
    io_set_new.c_cc[VTIME] = 0;

    // set new settings
    if (tcsetattr(m_file_descr, TCSANOW, &io_set_new) < 0)
    {
      m_status = -errno;
      SVH_LOG_DEBUG_STREAM("Serial",
                           "Serial(" << m_dev_name << ") Error>> tcsetattr failed. Status ("
                                     << m_status << ":" << strerror(-m_status));
      // DEBUGMSG(DD_SYSTEM, DL_DEBUG, "Serial(%s) Error>> tcsetattr failed. Status (%i:%s)\n",
      // m_dev_name, m_status, strerror(-m_status));
      return false;
    }
    else
      m_status = 0;

    if (m_serial_flags.getModemControlFlags() != SerialFlags::MCF_UNDEFINED)
    {
      SVH_LOG_DEBUG_STREAM("Serial",
                           "Serial(" << m_dev_name << ") setting hardware modem control flags to 0x"
                                     << m_serial_flags.getModemControlFlags());
      // DEBUGMSG(DD_SYSTEM, DL_DEBUG, "Serial(%s) setting hardware modem control flags to 0x%x\n",
      // m_dev_name, m_serial_flags.getModemControlFlags());
      int modem_control_flags = 0;
      if (m_serial_flags.getModemControlFlags() & SerialFlags::MCF_DTR)
      {
        modem_control_flags |= TIOCM_DTR;
      }
      if (m_serial_flags.getModemControlFlags() & SerialFlags::MCF_RTS)
      {
        modem_control_flags |= TIOCM_RTS;
      }

      ioctl(m_file_descr, TIOCMSET, modem_control_flags);
    }
  }

#elif defined _SYSTEM_WIN32_

  m_read_buffer_fill = 0;
  m_status           = 0;

  // Try to open the serial port.
  m_com = CreateFile(
    m_dev_name, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (m_com == INVALID_HANDLE_VALUE)
  {
    m_status = GetLastError();
    SVH_LOG_WARN_STREAM("Serial",
                        "Serial(" << m_dev_name << "): ERROR>> open port failed ("
                                  << StatusText().c_str() << ").");
    // WARNINGMSG("Serial(%s): ERROR>> open port failed (%s).\n", m_dev_name, StatusText().c_str());
  }

  // Set the receive and send buffer size.
  if (m_status == 0)
  {
    if (!SetupComm(m_com, 0x4000, 0x4000))
    {
      m_status = GetLastError();
      SVH_LOG_WARN_STREAM("Serial",
                          "Serial(" << m_dev_name << "): ERROR>> SetupComm failed ("
                                    << StatusText().c_str() << ").");
      // WARNINGMSG("Serial(%s): ERROR>> SetupComm failed (%s).\n", m_dev_name,
      // StatusText().c_str());
    }
  }

  // Get the current serial port configuration.
  DCB dcb;
  memset(&dcb, 0, sizeof(DCB));
  if (m_status == 0)
  {
    if (!GetCommState(m_com, &dcb))
    {
      m_status = GetLastError();
      SVH_LOG_WARN_STREAM("Serial",
                          "Serial(" << m_dev_name << "): ERROR>> GetCommState failed ("
                                    << StatusText().c_str() << ").");
      // WARNINGMSG("Serial(%s): ERROR>> GetCommState failed (%s).\n", m_dev_name,
      // StatusText().c_str());
    }
  }

  // Set the serial port configuration from the supplied seiral flags.
  if (m_status == 0)
  {
    m_serial_flags.GetDCB(&dcb);
    if (!SetCommState(m_com, &dcb))
    {
      m_status = GetLastError();
      SVH_LOG_WARN_STREAM("Serial",
                          "Serial(" << m_dev_name << "): ERROR>> SetCommState failed ("
                                    << StatusText().c_str() << ").");
      // WARNINGMSG("Serial(%s): ERROR>> SetCommState failed (%s).\n", m_dev_name,
      // StatusText().c_str());
      return false;
    }
  }
#endif

  return m_status == 0;
}

void Serial::dumpData(void* data, size_t length)
{
  unsigned char* c_data = static_cast<unsigned char*>(data);
  printf("Serial::DumpData: ");
  for (size_t i = 0; i < length; ++i)
  {
    printf("%02X ", int(c_data[i]));
  }
  printf("\n");
}

int Serial::changeBaudrate(SerialFlags::BaudRate speed)
{
  // Nothing to be done here.
  if (m_serial_flags.getBaudRate() == speed)
  {
    // success
    return 0;
  }
  m_serial_flags.setBaudRate(speed);

#if defined _SYSTEM_LINUX_
  {
    if (m_file_descr < 0)
      return m_status;

    struct termios io_set;

    // Get device settings
    if (tcgetattr(m_file_descr, &io_set) < 0)
    {
      m_status = -errno;
      SVH_LOG_DEBUG_STREAM("Serial",
                           "Serial(" << m_dev_name << "): Error>> tcgetattr failed. Status ("
                                     << m_status << ":" << strerror(-m_status));
      // DEBUGMSG(DD_SYSTEM, DL_DEBUG, "Serial Error>> tcgetattr (%s) failed. m_status (%i:%s)\n",
      // m_dev_name, m_status, strerror(-m_status));
    }
    else
    {
      // clear speed-settings
      io_set.c_cflag &= ~CBAUD;
      // add new speed-settings
      io_set.c_cflag |= SerialFlags::cFlags(speed);

      // set new device settings
      if (tcsetattr(m_file_descr, TCSANOW, &io_set) < 0)
      {
        m_status = -errno;
        SVH_LOG_DEBUG_STREAM("Serial",
                             "Serial(" << m_dev_name << "): Error>> tcsetattr failed. Status ("
                                       << m_status << ":" << strerror(-m_status));
        // DEBUGMSG(DD_SYSTEM, DL_DEBUG, "Serial(%s) Error>> tcsetattr failed. Status (%i:%s)\n",
        // m_dev_name, m_status, strerror(-m_status));
      }
      else
      {
        SVH_LOG_INFO_STREAM("Serial", "Serial:ChangeBaudrate " << speed << " successful.");
        // INFOMSG("Serial:ChangeBaudrate %i successful\n", speed);
        m_status = 0;
      }
    }
  }

  return m_status;
#endif

#ifdef _SYSTEM_DARWIN_
  SVH_LOG_WARN_STREAM("Serial", "Serial:changeBaudrate() >> to be implemented");
  // WARNINGMSG("Serial:changeBaudrate() >> to be implemented\n");
  return -1;
#endif

#ifdef _SYSTEM_WIN32_
  m_status = 0;

  // Clears the output and input buffers.
  if (!PurgeComm(m_com, PURGE_RXCLEAR) || !PurgeComm(m_com, PURGE_TXCLEAR))
  {
    m_status = GetLastError();
    SVH_LOG_WARN_STREAM("Serial",
                        "Serial(" << m_dev_name << "): ERROR>> PurgeComm failed ("
                                  << StatusText().c_str() << ").");
    // WARNINGMSG("Serial(%s): ERROR>> PurgeComm failed (%s).\n", m_dev_name, StatusText().c_str());
  }

  // Get the current serial port configuration.
  DCB dcb;
  memset(&dcb, 0, sizeof(DCB));
  if (m_status == 0)
  {
    if (!GetCommState(m_com, &dcb))
    {
      m_status = GetLastError();
      SVH_LOG_WARN_STREAM("Serial",
                          "Serial(" << m_dev_name << "): ERROR>> GetCommState failed ("
                                    << StatusText().c_str() << ").");
      // WARNINGMSG("Serial(%s): ERROR>> GetCommState failed (%s).\n", m_dev_name,
      // StatusText().c_str());
    }
  }

  // Set the serial port configuration with the new baud rate.
  if (m_status == 0)
  {
    m_serial_flags.GetDCB(&dcb);
    if (!SetCommState(m_com, &dcb))
    {
      m_status = GetLastError();
      SVH_LOG_WARN_STREAM("Serial",
                          "Serial(" << m_dev_name << "): ERROR>> SetCommState failed ("
                                    << StatusText().c_str() << ").");
      // WARNINGMSG("Serial(%s): ERROR>> SetCommState failed (%s).\n", m_dev_name,
      // StatusText().c_str());
    }
  }

  return -m_status;
#endif
}

int Serial::clearReceiveBuffer()
{
#ifdef _SYSTEM_WIN32_
  m_status = 0;

  // Clears the input buffers.
  if (PurgeComm(m_com, PURGE_RXCLEAR))
  {
    m_read_buffer_fill = 0;
  }
  else
  {
    m_status = GetLastError();
    SVH_LOG_WARN_STREAM("Serial",
                        "Serial(" << m_dev_name << "): ERROR>> PurgeComm failed ("
                                  << StatusText().c_str() << ").");
    // WARNINGMSG("Serial(%s): ERROR>> PurgeComm failed (%s).\n", m_dev_name, StatusText().c_str());
  }

  return m_status;
#elif defined _SYSTEM_LINUX_
  // could not test for LXRT device, so return -1 to be on the safe side
  if (tcflush(m_file_descr, TCIFLUSH) != 0)
  {
    SVH_LOG_WARN_STREAM("Serial", "tcflush failed :(");
    return -1;
  }
#else
  return -1;
#endif
  return 0;
}

int Serial::clearSendBuffer()
{
#ifdef _SYSTEM_WIN32_
  m_status = 0;

  // Clears the output buffers.
  if (PurgeComm(m_com, PURGE_TXCLEAR))
  {
    m_read_buffer_fill = 0;
  }
  else
  {
    m_status = GetLastError();
    SVH_LOG_WARN_STREAM("Serial",
                        "Serial(" << m_dev_name << "): ERROR>> PurgeComm failed ("
                                  << StatusText().c_str() << ").");
    // WARNINGMSG("Serial(%s): ERROR>> PurgeComm failed (%s).\n", m_dev_name, StatusText().c_str());
  }

  return m_status;
#elif defined _SYSTEM_LINUX_
  // could not test for LXRT device, so return -1 to be on the safe side
  if (tcflush(m_file_descr, TCOFLUSH) != 0)
  {
    SVH_LOG_WARN_STREAM("Serial", "tcflush failed :(");
    return -1;
  }
#else
  return -1;
#endif
  return 0;
}

ssize_t Serial::write(const void* data, ssize_t size)
{
#if defined _SYSTEM_LINUX_
  if (m_file_descr < 0)
    return m_status;

  ssize_t bytes_out = 0;

  {
    // just write it to device
    if ((bytes_out = ::write(m_file_descr, (char*)data, size)) < 0)
    {
      m_status = -errno;
      SVH_LOG_DEBUG_STREAM("Serial",
                           "Serial(" << m_dev_name << ":" << m_dev_name
                                     << "): Error on writing. Status (" << m_status << ":"
                                     << strerror(-m_status) << ").");
      // DEBUGMSG(DD_SYSTEM, DL_DEBUG, "Serial(%s) Error on writing. Status (%i:%s)\n", m_dev_name,
      // m_status, strerror(-m_status));
    }
    else
    {
      m_status = 0;
    }
  }

  return bytes_out;

#elif defined _SYSTEM_WIN32_

  assert(m_com != INVALID_HANDLE_VALUE);

  DWORD bytes_written;
  if (!WriteFile(m_com, data, size, &bytes_written, NULL))
  {
    m_status = GetLastError();
    SVH_LOG_WARN_STREAM("Serial",
                        "Serial(" << m_dev_name << "): ERROR>> could not write data ("
                                  << StatusText().c_str() << ").");
    // WARNINGMSG("Serial(%s): ERROR>> could not write data (%s).\n", m_dev_name,
    // StatusText().c_str());
  }
  else
  {
    m_status = 0;
#  ifdef SERIAL_DUMP_DATA
    DumpData(data, bytes_written);
#  endif
  }

  return (m_status == 0 ? bytes_written : -m_status);
#else
  return -1;
#endif
}

ssize_t Serial::read(void* data, ssize_t size, unsigned long time, bool return_on_less_data)
{
  // tTime end_time = tTime().FutureUSec(time);
  auto end_time = std::chrono::high_resolution_clock::now() + std::chrono::microseconds(time);

#if defined _SYSTEM_LINUX_
  if (m_file_descr < 0)
    return m_status;

  std::chrono::duration<double, std::milli> tz;
  fd_set fds;
  ssize_t bytes_read = 0;
  ssize_t bytes_read_inc;
  int select_return;
  char* buffer = (char*)data;

  // if (time <= 0)
  //   time = 1;
  // LDM("Serial(%s)::Read(%i) (time left %li us)\n", m_dev_name, size, time);

  m_status = 0;
  {
    // We wait max time
    do
    {
      tz = end_time - std::chrono::high_resolution_clock::now();
      // min 1 us, otherwise we will read nothing at all
      if (tz < std::chrono::microseconds(1))
      {
        tz = std::chrono::microseconds(1);
      }

      // LDM("Serial(%s) Check Read.\n", m_dev_name);

      FD_ZERO(&fds);
      FD_SET(m_file_descr, &fds);
      timeval select_timeout = {0,
                                std::chrono::duration_cast<std::chrono::microseconds>(tz).count()};
      // Look for received data:
      if ((select_return = select(FD_SETSIZE, &fds, 0, 0, &select_timeout)) > 0)
      {
        // LDM("Serial(%s) Select successful.\n", m_dev_name);
        if (return_on_less_data)
        {
          if ((bytes_read_inc = ::read(m_file_descr, &buffer[bytes_read], size - bytes_read)) < 0)
          {
            m_status = -errno;
            SVH_LOG_DEBUG_STREAM("Serial",
                                 "Error on reading '" << m_dev_name << "'. Status (" << m_status
                                                      << ":" << strerror(-m_status) << ")");
            // DEBUGMSG(DD_SYSTEM, DL_DEBUG, "Error on reading '%s'. Status (%i:%s)\n", m_dev_name,
            // m_status, strerror(-m_status));
            return m_status;
          }
          // Any bytes read ?
          if (bytes_read_inc > 0)
          {
            bytes_read += bytes_read_inc;
            // LDM("Serial(%s) Data read (%i => %i).\n", m_dev_name, bytes_read_inc, bytes_read);
            if (bytes_read == size)
            {
              return bytes_read;
            }
          }
        }
        else
        {
          // LDM("serial:time left %lu\n",Time2Long(tz));
          // Are there already enough bytes received ?
          if (ioctl(m_file_descr, FIONREAD, &bytes_read_inc) < 0)
          {
            m_status = -errno;
            SVH_LOG_DEBUG_STREAM("Serial",
                                 "Error on ioctl FIONREAD '" << m_dev_name << "'. Status ("
                                                             << m_status << ":"
                                                             << strerror(-m_status) << ")");
            // DEBUGMSG(DD_SYSTEM, DL_DEBUG, "Error on ioctl FIONREAD '%s'. Status (%i:%s)\n",
            // m_dev_name, m_status, strerror(-m_status));
            return m_status;
          }
          else
          {
            // Yes? then read data
            if (bytes_read_inc >= size)
            {
              if ((bytes_read = ::read(m_file_descr, buffer, size)) < 0)
              {
                m_status = -errno;
                SVH_LOG_DEBUG_STREAM("Serial",
                                     "Error on read '" << m_dev_name << "'. Status (" << m_status
                                                       << ":" << strerror(-m_status) << ")");
                // DEBUGMSG(DD_SYSTEM, DL_DEBUG, "Error on read '%s'. Status (%i:%s)\n", m_dev_name,
                // m_status, strerror(-m_status));
                return m_status;
              }
              else
              {
                // LDM("Serial(%s) Data read (%i bytes).\n", m_dev_name, bytes_read);
                return bytes_read;
              }
            }
            // No ? do nothing
          }
        }
      }
      else if (select_return < 0)
      {
        m_status = -errno;
        SVH_LOG_DEBUG_STREAM("Serial",
                             "Error on select '" << m_dev_name << "'. Status (" << m_status << ":"
                                                 << strerror(-m_status) << ")");
        // DEBUGMSG(DD_SYSTEM, DL_DEBUG, "Error on select '%s'. Status (%i:%s)\n", m_dev_name,
        // m_status, strerror(-m_status));
        return m_status;
      }
    }
    // Look again for data, if any time left
    while (std::chrono::high_resolution_clock::now() < end_time);
  }

  // LDM("Serial(%s)::Read(%i) (time left %li us) = %i.\n", m_dev_name, size, (end_time -
  // std::chrono::high_resolution_clock::now()).toUSec(), bytes_read);
  return bytes_read;
#endif


#ifdef _SYSTEM_DARWIN_
  SVH_LOG_WARN_STREAM("Serial", "Serial:Read() >> to be implemented!");
  // WARNINGMSG("Serial:Read() >> to be implemented\n");
  return 0;
#endif

#ifdef _SYSTEM_WIN32_
  assert(m_com != INVALID_HANDLE_VALUE);

  m_status = 0;

  // Set the timeout for the read operation.
  COMMTIMEOUTS timeout;
  timeout.ReadIntervalTimeout         = time / 1000;
  timeout.ReadTotalTimeoutMultiplier  = 1;
  timeout.ReadTotalTimeoutConstant    = time / 1000;
  timeout.WriteTotalTimeoutMultiplier = 1;
  timeout.WriteTotalTimeoutConstant   = MAXDWORD;
  if (!SetCommTimeouts(m_com, &timeout))
  {
    m_status = GetLastError();
    SVH_LOG_WARN_STREAM("Serial",
                        "Serial(" << m_dev_name << "): ERROR>> setting read timeout failed ("
                                  << StatusText().c_str() << ")");
    // WARNINGMSG("Serial(%s): ERROR>> setting read timeout failed (%s).\n", m_dev_name,
    // StatusText().c_str());
  }

  // Try to receive data.
  if (m_read_buffer_fill <= size && m_status == 0)
  {
    DWORD bytes_received   = 0;
    size_t bytes_remaining = (m_read_buffer_fill < size ? size - m_read_buffer_fill : 0);
    auto now std::chrono::high_resolution_clock::now();
    do
    {
      if (ReadFile(
            m_com, m_read_buffer + m_read_buffer_fill, bytes_remaining, &bytes_received, NULL))
      {
        m_read_buffer_fill += bytes_received;
        if (bytes_remaining > bytes_received)
        {
          bytes_remaining -= bytes_received;
        }
        else
        {
          bytes_remaining = 0;
        }
      }
      else
      {
        DWORD error = GetLastError();
        if (error != ERROR_TIMEOUT)
        {
          m_status = error;
          SVH_LOG_WARN_STREAM("Serial",
                              "Serial(" << m_dev_name << "): ERROR>> error during read ("
                                        << StatusText().c_str() << ")");
          // WARNINGMSG("Serial(%s): ERROR>> error during read (%s).\n", m_dev_name,
          // StatusText().c_str());
        }
      }
      now = std::chrono::high_resolution_clock::now();
    } while (m_status == 0 && !return_on_less_data && bytes_remaining && now < end_time);

    if (m_status == 0 && !return_on_less_data && bytes_remaining && now >= end_time)
    {
      m_status = ERROR_TIMEOUT;
      SVH_LOG_WARN_STREAM("Serial",
                          "Serial(" << m_dev_name << "): ERROR>> error during read ("
                                    << StatusText().c_str() << ")");
      // WARNINGMSG("Serial(%s): ERROR>> error during read (%s).\n", m_dev_name,
      // StatusText().c_str());
    }
  }

  // Copy data to output buffer.
  ssize_t bytes_received;
  if (m_status == 0)
  {
    bytes_received = std::min(size, m_read_buffer_fill);
    memcpy(data, m_read_buffer, bytes_received);

    // \todo Make this more efficient.
    for (ssize_t write_index = 0, read_index = bytes_received; read_index < m_read_buffer_fill;
         ++write_index, ++read_index)
    {
      m_read_buffer[write_index] = m_read_buffer[read_index];
    }
    m_read_buffer_fill -= bytes_received;

#  ifdef SERIAL_DUMP_DATA
    DumpData(data, bytes_received);
#  endif
  }

  return (m_status == 0 ? bytes_received : -m_status);
#endif
}

std::string Serial::statusText() const
{
#if defined _SYSTEM_LINUX_
  return strerror(-m_status);
#elif defined _SYSTEM_WIN32_
  LPVOID msg_buff = NULL;
  DWORD res       = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                              FORMAT_MESSAGE_IGNORE_INSERTS,
                            NULL,
                            m_status, //  from GetLastError(),
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                            (LPTSTR)&msg_buff,
                            0,
                            NULL);
  std::string result;
  if (res)
  {
    result = (LPCTSTR)msg_buff;
    LocalFree(msg_buff);
  }
  else
  {
    result = "fatal internal error";
  }
  return result;
#else
  return "Plattform not supported!";
#endif
}

bool Serial::isOpen() const
{
#ifdef _SYSTEM_LINUX_
  return m_file_descr >= 0;
#elif defined _SYSTEM_WIN32_
  return m_com != INVALID_HANDLE_VALUE;
#else
  return false;
#endif
}

void Serial::close()
{
  // LDM("Serial::Close\n");
#ifdef _SYSTEM_LINUX_
  if (m_file_descr >= 0)
  {
    {
      // restore old setting
      if (tcsetattr(m_file_descr, TCSANOW, &m_io_set_old) < 0)
      {
        m_status = -errno;
        SVH_LOG_DEBUG_STREAM("Serial",
                             "Serial(" << m_dev_name << ") Error>> tcsetattr failed. Status ("
                                       << m_status << ":" << strerror(-m_status) << ")");
        // DEBUGMSG(DD_SYSTEM, DL_CREATE, "Serial(%s) Error>> tcsetattr failed. Status (%i:%s)\n",
        // m_dev_name, m_status, strerror(-m_status));
      }
      // close device
      if (::close(m_file_descr) < 0)
      {
        m_status = -errno;
        SVH_LOG_DEBUG_STREAM("Serial",
                             "Serial>> Error closing serial " << m_dev_name << ". Status ("
                                                              << m_status << ":"
                                                              << strerror(-m_status) << ")");
        // DEBUGMSG(DD_SYSTEM, DL_CREATE, "Serial>> Error closing serial %s. Status(%i:%s)\n",
        // m_dev_name, m_status, strerror(-m_status));
      }
    }

    m_file_descr = -1;
  }
#endif

#ifdef _SYSTEM_WIN32_
  if (m_com != INVALID_HANDLE_VALUE)
  {
    CloseHandle(m_com); // close device
    m_com = INVALID_HANDLE_VALUE;
  }
#endif
  m_status = 0;
  // LDM("Serial::Close done.\n");
}

Serial::~Serial()
{
  // LDM("~Serial start\n");

  close();
  //    free(m_dev_name);
  m_dev_name = NULL;

  // LDM("~Serial done\n");
}


} // namespace serial
} // namespace driver_svh
