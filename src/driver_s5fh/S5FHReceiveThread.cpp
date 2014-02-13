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

#include <icl_core/BaseTypes.h>
#include <icl_core/Vector.h>

using icl_comm::ArrayBuilder;

namespace driver_s5fh {

S5FHReceiveThread::S5FHReceiveThread(const TimeSpan& period, Serial *device,
                                     ReceivedPacketCallback const & received_callback)
  : PeriodicThread("S5FHReceiveThread", period),
    m_serial_device(device),
    m_received_state(eRS_HEADER1),
    m_length(0),
    m_data(0, 0),
    m_ab(0),
    m_packets_received(0),
    m_received_callback(received_callback)
{
}

void S5FHReceiveThread::run()
{
  while (execute())
  {
    if (m_serial_device != NULL)
    {
      if (m_serial_device->IsOpen())
      {
        receiveData();
      }
      else
      {
        LOGGING_WARNING_C(DriverS5FH, S5FHReceiveThread, "Cannot read data from serial device. It is not opened!" << endl);
      }
    }

    // Wait for the thread period so that the timing is in sync.
    waitPeriod();
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
      // Reset Array Builder
      m_ab.reset(0);

      // read index data byte
      uint8_t index = 0;
      if (m_serial_device->Read(&index, sizeof(uint8_t)))
      {
        m_ab.appendWithoutConversion(index);
        m_received_state = eRS_ADDRESS;
      }
      break;
    }
    case eRS_ADDRESS:
    {
      // read address data byte
      uint8_t address = 0;
      if (m_serial_device->Read(&address, sizeof(uint8_t)))
      {
        m_ab.appendWithoutConversion(address);
        m_received_state = eRS_LENGTH;
      }
      break;
    }
    case eRS_LENGTH:
    {
      // read data length
      uint16_t length = 0;
      if (m_serial_device->Read(&length, sizeof(uint16_t)))
      {
        m_ab.appendWithoutConversion(length);
        m_length = m_ab.readBack<u_int16_t>();
        m_received_state = eRS_DATA;
      }
      break;
    }
    case eRS_DATA:
    {
      // read received data
      uint8_t buffer[m_length];
      if (m_serial_device->Read(reinterpret_cast<void *>(buffer), m_length))
      {
        m_data.clear();
        m_data.insert(m_data.end(), &buffer[0], &buffer[m_length]);

        m_ab.appendWithoutConversion(m_data);
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
        // probe for correct checksum
        for (size_t i = 0; i < m_data.size(); i++)
        {
          checksum1 -= m_data[i];
          checksum2 ^= m_data[i];
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
      // start with an empty package
      // Warning: It is imperative for correct readouts to create the received_packet with the correct length!
      S5FHSerialPacket received_packet(m_length);
      m_ab >> received_packet;

      m_packets_received++;

      //! packet received callback function
      if (m_received_callback)
      {
        m_received_callback(received_packet, m_packets_received);
      }

      m_received_state = eRS_HEADER1;
      break;
    }
  }

  return true;
}

}
