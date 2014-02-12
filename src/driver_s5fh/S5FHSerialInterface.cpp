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

#include <icl_comm/ByteOrderConversion.h>

using icl_core::TimeSpan;
using icl_comm::serial::SerialFlags;

namespace driver_s5fh {

S5FHSerialInterface::S5FHSerialInterface(const std::string &dev_name, ReceivedPacketCallback const & received_packet_callback)
{
  // create and open serial device
  m_serial_device = new Serial(dev_name.c_str(), SerialFlags(SerialFlags::eBR_921600, SerialFlags::eDB_8));

  if (m_serial_device != NULL)
  {
    if (!m_serial_device->Open())
    {
      LOGGING_ERROR_C(DriverS5FH, S5FHSerialInterface, "Could not open serial device: " << dev_name.c_str() << endl);
    }
  }

  // create and start receive thread
  m_receive_thread = new S5FHReceiveThread(TimeSpan::createFromMSec(10), m_serial_device, received_packet_callback);

  if (m_receive_thread != NULL)
  {
    if (!m_receive_thread->start())
    {
      LOGGING_ERROR_C(DriverS5FH, S5FHSerialInterface, "Could not start the receive thread for the serial device!" << endl);
    }
  }

  // initialize packet counter
  m_packets_transmitted = 0;
}

S5FHSerialInterface::~S5FHSerialInterface()
{
  close();
}

void S5FHSerialInterface::close()
{
  // cancel and delete receive packet thread
  m_receive_thread->cancel();
  delete m_receive_thread;

  // close and delete serial device handler
  m_serial_device->Close();
  delete m_serial_device;
}

bool S5FHSerialInterface::sendPacket(S5FHSerialPacket& packet)
{
  uint8_t check_sum1 = 0;
  uint8_t check_sum2 = 0;

  for (size_t i; i < packet.data.size(); i++)
  {
    check_sum1 += packet.data[i];
    check_sum2 ^= packet.data[i];
  }

  // set packet counter
  packet.index = m_packets_transmitted;

  if (m_serial_device->IsOpen())
  {
    // serialize packet
    size_t size = packet.data.size() + cPACKET_APPENDIX_SIZE;
    icl_comm::ArrayBuilder send_array(size);
    send_array << PACKET_HEADER1 << PACKET_HEADER2 << packet << check_sum1 << check_sum2;

    m_serial_device->Write(send_array.array.data(), size);
  }
  else
  {
    return false;
  }

  m_packets_transmitted++;

  return true;
}

}
