// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Lars pfotzer
 * \author  Georg Heppner
 * \date    2014-01-30
 * \date    2014-07-16
 *
 * This file contains the S5FHSerialInterface class that is used to
 * handle the protocoll overhead of the serial communication.
 * It uses an icl_comm serial device that opens the physical connection and
 * is responsible to manage this hardware resource as well as protocoll issues
 * like sync bytes, checksum calculation and counting of packets send and received.
 */
//----------------------------------------------------------------------
#ifndef DRIVER_S5FH_S5FH_SERIAL_INTERFACE_H_INCLUDED
#define DRIVER_S5FH_S5FH_SERIAL_INTERFACE_H_INCLUDED

#include <driver_s5fh/ImportExport.h>
#include <driver_s5fh/S5FHSerialPacket.h>
#include <driver_s5fh/S5FHReceiveThread.h>

// Hardware interface
#include <icl_comm_serial/Serial.h>
using icl_comm::serial::Serial;

namespace driver_s5fh {

/*!
 *  \brief Basic communication handler for the SCHUNK five finger hand.
 */
class DRIVER_S5FH_IMPORT_EXPORT S5FHSerialInterface
{
public:
  //!
  //! \brief Constructs a serial interface class for basic communication with the SCHUNK five finger hand.
  //! \param received_packet_callback function to call whenever a packet was received
  //!
  S5FHSerialInterface(const ReceivedPacketCallback &received_packet_callback);

  //! Default DTOR
  ~S5FHSerialInterface();

  //!
  //! \brief connecting to serial device and starting receive thread
  //! \param dev_name Filehandle of the device i.e. dev/ttyUSB0
  //! \return bool true if connection was succesfull
  //!
  bool connect(const std::string &dev_name);

  //!
  //! \brief canceling receive thread and closing connection to serial port
  //!
  void close();

  //!
  //! \brief returns connected state of serial device
  //! \return bool true if the device is connected
  //!
  bool isConnected() { return m_connected; }

  //!
  //! \brief function for sending packets via serial device to the S5FH
  //! \param packet the prepared Serial Packet
  //! \return true if successful
  //!
  bool sendPacket(S5FHSerialPacket &packet);

  //!
  //! \brief get number of transmitted packets
  //! \return number of successfully sent packets
  //!
  unsigned int transmittedPacketCount() { return m_packets_transmitted; }

  /*!
   * \brief resetTransmitPackageCount Resets the transmitpackage count to zero
   */
  void resetTransmitPackageCount();

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
  unsigned int m_packets_transmitted;
};

}

#endif
