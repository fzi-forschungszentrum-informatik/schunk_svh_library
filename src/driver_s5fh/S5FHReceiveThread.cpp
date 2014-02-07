// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Lars Pfotzer
 * \date    2014-02-03
 *
 */
//----------------------------------------------------------------------
#include <driver_s5fh/S5FHReceiveThread.h>
#include <driver_s5fh/Logging.h>

#include <icl_comm/ByteOrderConversion.h>
#include <icl_core/BaseTypes.h>
#include <icl_core/Vector.h>

namespace driver_s5fh {

S5FHReceiveThread::S5FHReceiveThread(const TimeSpan& period, S5FHSerialInterface* interface, Serial *device)
  : PeriodicThread("S5FHReceiveThread", period),
    m_serial_interface(interface),
    m_serial_device(device),
    m_received_state(eRS_HEADER1),
    m_received_packet(NULL),
    m_packets_received(0)
{
}

void S5FHReceiveThread::run()
{
  // Wait for the first period so that the timing is in sync.
  waitPeriod();

  if (m_serial_device->IsOpen())
  {
    receiveData();
  }
  else
  {
    LOGGING_WARNING_C(DriverS5FH, S5FHReceiveThread, "Cannot read data from serial device. It is not opened!" << endl);
  }
}

bool S5FHReceiveThread::receiveData()
{
  switch (m_received_state)
  {
    case eRS_HEADER1:
    {
      uint8_t data_byte = 0;
      if (m_serial_device->Read(&data_byte, sizeof(uint8_t)))
      {
        if (data_byte == PACKET_HEADER1)
        {
          m_received_state = eRS_HEADER2;
        }
      }
      break;
    }
    case eRS_HEADER2:
    {
      uint8_t data_byte = 0;
      if (m_serial_device->Read(&data_byte, sizeof(uint8_t)))
      {
        switch (data_byte)
        {
          case PACKET_HEADER2:
          {
            m_received_state = eRS_INDEX;
            break;
          }
          case PACKET_HEADER1:
          {
            m_received_state = eRS_HEADER2;
            break;
          }
          default:
          {
            m_received_state = eRS_HEADER1;
            break;
          }
        }
      }
      break;
    }
    case eRS_INDEX:
    {
      // start with a new empty package
      m_received_packet = new S5FHSerialPacket();

      uint8_t data_byte = 0;
      if (m_serial_device->Read(&data_byte, sizeof(uint8_t)))
      {
        m_received_packet->index = data_byte;
        m_received_state = eRS_ADDRESS;
      }
      break;
    }
    case eRS_ADDRESS:
    {
      uint8_t data_byte = 0;
      if (m_serial_device->Read(&data_byte, sizeof(uint8_t)))
      {
        m_received_packet->address = data_byte;
        m_received_state = eRS_LENGTH;
      }
      break;
    }
    case eRS_LENGTH:
    {
      uint16_t length = 0;
      if (m_serial_device->Read(&(length), sizeof(uint16_t)))
      {
        length = 40;
        m_received_packet->data = std::vector<uint8_t>(length, 0);
        m_received_state = eRS_DATA;
      }
      break;
    }
    case eRS_DATA:
    {
      if (m_serial_device->Read(&(m_received_packet->data), m_received_packet->data.size()))
      {
        m_received_state = eRS_CHECKSUM;
      }
      break;
    }
    case eRS_CHECKSUM:
    {
      uint8_t checksum1 = 0;
      uint8_t checksum2 = 0;
      if (m_serial_device->Read(&checksum1, sizeof(uint8_t))
          && m_serial_device->Read(&checksum2, sizeof(uint8_t)))
      {
        for (size_t i = 0; i < m_received_packet->data.size(); i++)
        {
          checksum1 -= m_received_packet->data[i];
          checksum2 ^= m_received_packet->data[i];
        }

        if ((checksum1 == 0) && (checksum2 == 0))
        {
          m_received_state = eRS_COMPLETE;
        }
        else
        {
          m_received_state = eRS_HEADER1;
        }
      }
      break;
    }
    case eRS_COMPLETE:
    {
      m_packets_received++;

      //TODO: Add callback function to S5FH Controller and call from here:
//      SerialPacketReceivedEventHandler handler = SerialPacketReceived;
//      if (handler != null)
//        handler(this, packet);
      m_received_state = eRS_HEADER1;
      break;
    }
  }
  return true;
}

}
