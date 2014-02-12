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

S5FHSerialInterface::S5FHSerialInterface(ReceivedPacketCallback const & received_packet_callback)
{
  // create serial device
  m_serial_device = NULL;

  // create receive thread
  m_receive_thread = new S5FHReceiveThread(TimeSpan::createFromMSec(10), m_serial_device, received_packet_callback);

  // initialize packet counter
  m_packets_transmitted = 0;
}

S5FHSerialInterface::~S5FHSerialInterface()
{
  close();
}

bool S5FHSerialInterface::connect(const std::string &dev_name)
{
  // create serial device
  m_serial_device = new Serial(dev_name.c_str(), SerialFlags(SerialFlags::eBR_921600, SerialFlags::eDB_8));

  // open serial device
  if (m_serial_device != NULL)
  {
    if (!m_serial_device->Open())
    {
      LOGGING_ERROR_C(DriverS5FH, S5FHSerialInterface, "Could not open serial device: " << dev_name.c_str() << endl);
      close();
      return false;
    }
  }

  // start receive thread
  if (m_receive_thread != NULL)
  {
    if (!m_receive_thread->start())
    {
      LOGGING_ERROR_C(DriverS5FH, S5FHSerialInterface, "Could not start the receive thread for the serial device!" << endl);
      close();
      return false;
    }
  }

  return true;
}

void S5FHSerialInterface::close()
{
  // cancel and delete receive packet thread
  if (m_receive_thread != NULL)
  {
    // FIXME: PLEASE FIX ME (ask JAN) !!!
    m_receive_thread->stop();
    m_receive_thread->join();
    delete m_receive_thread;
  }

  // close and delete serial device handler
  if (m_serial_device != NULL)
  {
    m_serial_device->Close();
    delete m_serial_device;
  }
}

bool S5FHSerialInterface::sendPacket(S5FHSerialPacket& packet)
{
  if (m_serial_device != NULL)
  {
    uint8_t check_sum1 = 0;
    uint8_t check_sum2 = 0;

    for (size_t i = 0; i < packet.data.size(); i++)
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

      size_t bytes_send = 0;
      while (bytes_send < size)
      {
        bytes_send += m_serial_device->Write(send_array.array.data() + bytes_send, size - bytes_send);
      }

      icl_core::os::usleep(500);

    }
    else
    {
      return false;
    }

    m_packets_transmitted++;
  }

  return true;
}

}
