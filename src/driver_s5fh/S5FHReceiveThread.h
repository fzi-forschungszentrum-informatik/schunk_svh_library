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
#ifndef DRIVER_S5FH_S5FH_RECEIVE_THREAD_H_INCLUDED
#define DRIVER_S5FH_S5FH_RECEIVE_THREAD_H_INCLUDED

#include <icl_core/TimeSpan.h>
#include <icl_core_thread/PeriodicThread.h>
#include <icl_comm_serial/Serial.h>
#include <icl_comm/ByteOrderConversion.h>

#include <driver_s5fh/Logging.h>
#include <driver_s5fh/S5FHSerialPacket.h>

#include <boost/function.hpp>

using icl_core::TimeSpan;
using icl_core::thread::PeriodicThread;
using icl_comm::serial::Serial;

namespace driver_s5fh {

//! definition of boost function callback for received packages
typedef boost::function<void (const S5FHSerialPacket& packet, unsigned int packet_count)> ReceivedPacketCallback;

/*! Thread for periodically receiving messages from the serial device.
 */
class S5FHReceiveThread : public PeriodicThread
{
public:
  S5FHReceiveThread(const TimeSpan& period, Serial* device,
                    ReceivedPacketCallback const & received_callback);

  virtual ~S5FHReceiveThread();

  //! run method of the thread
  virtual void run();

  //! received packets count getter
  unsigned int receivedPacketCount() { return m_packets_received; }

private:

  //! pointer to serial device object
  Serial* m_serial_device;

  //! enum for receive packet state machine
  enum
  {
    eRS_HEADER1,
    eRS_HEADER2,
    eRS_INDEX,
    eRS_ADDRESS,
    eRS_LENGTH,
    eRS_DATA,
    eRS_CHECKSUM,
    eRS_COMPLETE
  } typedef tState;

  //! current state of the state machine
  tState m_received_state;

  //! length of received serial data
  u_int16_t m_length;

  //! length of received serial data
  std::vector<u_int8_t> m_data;

  //! pointer to array builder object for packet receive
  icl_comm::ArrayBuilder* m_ab;

  //! packets counter
  unsigned int m_packets_received;

  //! state machine processing received data
  bool receiveData();

  //! function callback for received packages
  ReceivedPacketCallback m_received_callback;

};

}

#endif
