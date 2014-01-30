// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Lars Pfotzer
 * \date    2014-01-30
 *
 */
//----------------------------------------------------------------------
#include "driver_s5fh/S5FHSerialInterface.h"

#include "driver_s5fh/Logging.h"

using icl_core::TimeSpan;

namespace driver_s5fh {

S5FHSerialInterface::S5FHSerialInterface(const std::string &dev_name)
{
  m_serial_device = new Serial(dev_name.c_str(), SerialFlags(SerialFlags::eBR_921600, SerialFlags::eDB_8));
  m_receive_thread = new S5FHReceiveThread(TimeSpan::createFromMSec(10), m_serial_device);

  if (m_serial_device != NULL)
  {
    if (!m_serial_device->Open())
    {
      LOGGING_ERROR_C(DriverS5FH, S5FHSerialInterface, "Could not open serial device: " << dev_name.c_str() << endl);
    }
  }

  if (m_receive_thread != NULL)
  {
    if (!m_receive_thread->start())
    {
      LOGGING_ERROR_C(DriverS5FH, S5FHSerialInterface, "Could not start the receive thread for the serial device!" << endl);
    }
  }
}

S5FHSerialInterface::~S5FHSerialInterface()
{
  m_receive_thread->cancel();
  m_serial_device->Close();

  delete m_receive_thread;
  delete m_serial_device;
}

}
