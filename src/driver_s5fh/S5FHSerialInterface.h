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
  /*! Constructs a serial interface class for basic communication with the SCHUNK five finger hand.
   */
  S5FHSerialInterface(const std::string &dev_name);

  ~S5FHSerialInterface();

  //! function for sending packets via serial device to the S5FH
  bool sendPacket(const S5FHSerialPacket& packet);

  //! transmitted packets count getter
  unsigned int transmittedPacketCount() { return m_packets_transmitted; }

  //! received packets count getter
  unsigned int receivedPacketCount() { return m_packets_received; }

  //! increase received counter
  void increaseReceivedCounter() { m_packets_received++; }

protected:

  //! pointer to serial interface object
  Serial *m_serial_device;

  //! packet counters
  unsigned int m_packets_received;
  unsigned int m_packets_transmitted;

  //! cecksum calculation
  void calcCheckSum(uint8_t &check_sum1, uint8_t &check_sum2, const S5FHSerialPacket& packet);

private:

  //! thread for receiving serial packets
  S5FHReceiveThread *m_receive_thread;

};

}

#endif
