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
 * \date    2014-02-03
 * \date    2014-07-16
 *
 * This file contains the ReceiveThread for the serial communication.
 * In order to receive packages independently from the sending direction
 * this thread periodically polls the serial interface for new data. If data
 * is present a statemachine will evaluate the right packet structure and send the
 * data to further parsing once a complete serial packaged is received
 */
//----------------------------------------------------------------------
#include <chrono>
#include <schunk_svh_library/Logger.h>
#include <schunk_svh_library/serial/SVHReceiveThread.h>
#include <sstream>
#include <thread>

using driver_svh::ArrayBuilder;

namespace driver_svh {

SVHReceiveThread::SVHReceiveThread(const std::chrono::microseconds& idle_sleep,
                                   std::shared_ptr<Serial> device,
                                   ReceivedPacketCallback const& received_callback)
  : m_idle_sleep(idle_sleep)
  , m_serial_device(device)
  , m_received_state(RS_HEADE_R1)
  , m_length(0)
  , m_data(0, 0)
  , m_ab(0)
  , m_packets_received(0)
  , m_skipped_bytes(0)
  , m_received_callback(received_callback)
{
}

void SVHReceiveThread::run()
{
  while (m_continue)
  {
    if (m_serial_device) // != NULL)
    {
      if (m_serial_device->isOpen())
      {
        auto start = std::chrono::high_resolution_clock::now();

        // All we every want to do is receiving data :)
        if (!receiveData())
        {
          auto elapsed_time = std::chrono::high_resolution_clock::now() - start;

          if ((m_idle_sleep - elapsed_time).count() > 0) // sleep remainder of the cycle
          {
            std::this_thread::sleep_for(m_idle_sleep - elapsed_time);
          }
          else // We exceeded at least one cycle time. Sleep until we are back in sync.
          {
            std::this_thread::sleep_for(elapsed_time % m_idle_sleep);
          }
        }
      }
      else
      {
        SVH_LOG_WARN_STREAM("SVHReceiveThread",
                            "Cannot read data from serial device. It is not opened!");
        std::this_thread::sleep_for(m_idle_sleep); // we can neglect the processing time to get here
      }
    }
    else
    {
      // Wait for the thread period so that the timing is in sync.
      std::this_thread::sleep_for(m_idle_sleep); // we can neglect the processing time to get here
    }
  }
}

bool SVHReceiveThread::receiveData()
{
  /*
   * Each packet has to follow the defined packet structure which is ensured by the following state
   * machine. The "Bytestream" (not realy a stream) is interpreted byte by byte. If the structure is
   * still right the next state is entered, if a wrong byte is detected the whole packet is
   * discarded and the SM switches to the synchronization state aggain. If the SM reaches the final
   * state the packet will be given to the packet handler to decide what to do with its content.
   *  NOTE: All layers working with a SerialPacket (except this one) assume that the packet has a
   * valid structure and all data fields present.
   */
  uint8_t data_byte;
  ssize_t bytes = m_serial_device->read(&data_byte, sizeof(uint8_t));
  if (bytes < 0)
  {
    SVH_LOG_DEBUG_STREAM("SVHReceiveThread", "Serial read error:" << bytes);
    return false;
  }
  if (bytes < 1)
  {
    return false;
  }

  switch (m_received_state)
  {
    case RS_HEADE_R1: {
      if (data_byte == PACKET_HEADER1)
      {
        m_received_state = RS_HEADE_R2;
      }
      else
      {
        m_skipped_bytes++;
      }
      break;
    }
    case RS_HEADE_R2: {
      switch (data_byte)
      {
        case PACKET_HEADER2: {
          m_received_state = RS_INDEX;
          break;
        }
        case PACKET_HEADER1: {
          m_received_state = RS_HEADE_R2;
          m_skipped_bytes++;
          break;
        }
        default: {
          m_received_state = RS_HEADE_R1;
          m_skipped_bytes += 2;
          break;
        }
      }
      break;
    }
    case RS_INDEX: {
      // Reset Array Builder for each fresh packet
      m_ab.reset(0);

      // Data bytes are not cenverted in endianess at this point
      m_ab.appendWithoutConversion(data_byte);
      m_received_state = RS_ADDRESS;
      break;
    }
    case RS_ADDRESS: {
      // get the address
      m_ab.appendWithoutConversion(data_byte);
      m_received_state = RS_LENGT_H1;
      break;
    }
    case RS_LENGT_H1: {
      // get payload length
      m_ab.appendWithoutConversion(data_byte);
      m_received_state = RS_LENGT_H2;
      break;
    }
    case RS_LENGT_H2: {
      // get payload length
      m_ab.appendWithoutConversion(data_byte);
      m_length         = m_ab.readBack<uint16_t>();
      m_received_state = RS_DATA;
      m_data.clear();
      m_data.reserve(m_length);
      break;
    }
    case RS_DATA: {
      // get the payload itself
      // Some conversion due to legacy hardware calls
      m_data.push_back(data_byte);
      m_ab.appendWithoutConversion(data_byte);
      if (m_data.size() >= m_length)
      {
        m_received_state = RS_CHECKSU_M1;
      }
      break;
    }
    case RS_CHECKSU_M1: {
      m_checksum1      = data_byte;
      m_checksum2      = 0;
      m_received_state = RS_CHECKSU_M2;
      break;
    }
    case RS_CHECKSU_M2: {
      m_checksum2       = data_byte;
      uint8_t checksum1 = m_checksum1;
      uint8_t checksum2 = m_checksum2;
      // probe for correct checksum
      for (size_t i = 0; i < m_data.size(); ++i)
      {
        checksum1 -= m_data[i];
        checksum2 ^= m_data[i];
      }

      if ((checksum1 == 0) && (checksum2 == 0))
      {
        // start with an empty package
        // Warning: It is imperative for correct readouts to create the received_packet with the
        // correct length!
        SVHSerialPacket received_packet(m_length);
        m_ab >> received_packet;

        m_packets_received++;

        if (m_skipped_bytes > 0)
          SVH_LOG_DEBUG_STREAM("SVHReceiveThread", "Skipped " << m_skipped_bytes << " bytes ");
        SVH_LOG_DEBUG_STREAM("SVHReceiveThread",
                             "Received packet index:" << received_packet.index
                                                      << ", address:" << received_packet.address
                                                      << ", size:" << received_packet.data.size());
        m_skipped_bytes = 0;

        // notify whoever is waiting for this
        if (m_received_callback)
        {
          m_received_callback(received_packet, m_packets_received);
        }

        m_received_state = RS_HEADE_R1;
      }
      else
      {
        m_received_state = RS_HEADE_R1;

        SVHSerialPacket received_packet(m_length);
        m_ab >> received_packet;

        if (m_skipped_bytes > 0)
          SVH_LOG_DEBUG_STREAM("SVHReceiveThread", "Skipped " << m_skipped_bytes << " bytes: ");
        SVH_LOG_DEBUG_STREAM("SVHReceiveThread",
                             "Checksum error: " << (int)checksum1 << "," << (int)checksum2
                                                << "!=0, skipping " << m_length + 8
                                                << "bytes, packet index:" << received_packet.index
                                                << ", address:" << received_packet.address
                                                << ", size:" << received_packet.data.size());
        m_skipped_bytes = 0;
        if (m_received_callback)
        {
          m_received_callback(received_packet, m_packets_received);
        }
      }
      break;
    }
  }

  return true;
}

} // namespace driver_svh
