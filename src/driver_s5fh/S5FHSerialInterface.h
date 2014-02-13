// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Lars pfotzer
 * \date    2014-01-30
 *
 */
//----------------------------------------------------------------------
#ifndef DRIVER_S5FH_S5FH_SERIAL_INTERFACE_H_INCLUDED
#define DRIVER_S5FH_S5FH_SERIAL_INTERFACE_H_INCLUDED

#include <driver_s5fh/ImportExport.h>
#include <driver_s5fh/S5FHSerialPacket.h>
#include <driver_s5fh/S5FHReceiveThread.h>

#include <icl_comm_serial/Serial.h>

using icl_comm::serial::Serial;

namespace driver_s5fh {

/*! Basic communication with the SCHUNK five finger hand.
 */
class DRIVER_S5FH_IMPORT_EXPORT S5FHSerialInterface
{
public:
  //!
  //! \brief Constructs a serial interface class for basic communication with the SCHUNK five finger hand.
  //! \param received_packet_callback
  //!
  S5FHSerialInterface(const ReceivedPacketCallback &received_packet_callback);

  ~S5FHSerialInterface();

  //!
  //! \brief connecting to serial device and starting receive thread
  //! \param dev_name
  //! \return bool
  //!
  bool connect(const std::string &dev_name);

  //!
  //! \brief canceling receive thread and closing connection to serial port
  //!
  void close();

  //!
  //! \brief returns connected state of serial device
  //! \return bool
  //!
  bool isConnected() { return m_connected; }

  //!
  //! \brief function for sending packets via serial device to the S5FH
  //! \param packet
  //! \return
  //!
  bool sendPacket(S5FHSerialPacket &packet);

  //!
  //! \brief transmitted packets count getter
  //! \return
  //!
  unsigned int transmittedPacketCount() { return m_packets_transmitted; }

private:

  //! serial device connected state
  bool m_connected;

  //! pointer to serial interface object
  Serial *m_serial_device;

  //! cecksum calculation
  void calcCheckSum(uint8_t &check_sum1, uint8_t &check_sum2, const S5FHSerialPacket& packet);

  //! thread for receiving serial packets
  S5FHReceiveThread *m_receive_thread;

  //! Callback function for received packets
  ReceivedPacketCallback m_received_packet_callback;

  //! packet counters
  u_int8_t m_packets_transmitted;
};

}

#endif
