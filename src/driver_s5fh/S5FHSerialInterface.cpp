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

using namespace icl_comm::serial;

namespace driver_s5fh {

S5FHSerialInterface::S5FHSerialInterface(const std::string &dev_name)
{
  m_serial_device = new Serial(dev_name.c_str(), SerialFlags(SerialFlags::eBR_921600, SerialFlags::eDB_8));

  if (m_serial_device != NULL)
  {
    m_serial_device->Open();
  }
}

S5FHSerialInterface::~S5FHSerialInterface()
{
  m_serial_device->Close();

  delete m_serial_device;
}

}
