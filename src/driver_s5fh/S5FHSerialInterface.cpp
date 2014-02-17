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

S5FHSerialInterface::S5FHSerialInterface(ReceivedPacketCallback const & received_packet_callback) :
  m_connected(false),
  m_serial_device(NULL),
  m_receive_thread(NULL),
  m_received_packet_callback(received_packet_callback),
  m_packets_transmitted(0)
{
}

S5FHSerialInterface::~S5FHSerialInterface()
{
  close();
}

bool S5FHSerialInterface::connect(const std::string &dev_name)
{
  // close device if already opened
  close();

  // create serial device
  m_serial_device = new Serial(dev_name.c_str(), SerialFlags(SerialFlags::eBR_921600, SerialFlags::eDB_8));

  if (m_serial_device != NULL)
  {
    // open serial device
    if (!m_serial_device->Open())
    {
      LOGGING_ERROR_C(DriverS5FH, S5FHSerialInterface, "Could not open serial device: " << dev_name.c_str() << endl);
      return false;
    }
  }
  else
  {
    return false;
  }

  // create receive thread
  m_receive_thread = new S5FHReceiveThread(TimeSpan(0, 500000), m_serial_device, m_received_packet_callback);

  if (m_receive_thread != NULL)
  {
    // start receive thread
    if (!m_receive_thread->start())
    {
      LOGGING_ERROR_C(DriverS5FH, S5FHSerialInterface, "Could not start the receive thread for the serial device!" << endl);
      return false;
    }
  }
  else
  {
    return false;
  }

  m_connected = true;

  return true;
}

void S5FHSerialInterface::close()
{
  m_connected = false;

  // cancel and delete receive packet thread
  if (m_receive_thread != NULL)
  {
    // wait until thread has stopped
    m_receive_thread->stop();
    m_receive_thread->join();

    delete m_receive_thread;
    m_receive_thread = NULL;
  }

  // close and delete serial device handler
  if (m_serial_device != NULL)
  {
    m_serial_device->Close();

    delete m_serial_device;
    m_serial_device = NULL;
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

      icl_core::os::usleep(20000);

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
