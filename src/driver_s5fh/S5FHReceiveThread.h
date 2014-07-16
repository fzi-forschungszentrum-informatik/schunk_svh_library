// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Lars Pfotzer
 * \date    2014-01-30
 * \date    2014-07-16
 *
 * This file contains the ReceiveThread for the serial communication.
 * In order to receive packages independently from the sending direction
 * this thread periodically polls the serial interface for new data. If data
 * is present a statemachine will evaluate the right packet structure and send the
 * data to further parsing once a complete serial packaged is received
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

/*!
 * \brief Thread for receiving messages from the serial device.
 */
class S5FHReceiveThread : public PeriodicThread
{
public:
  /*!
   * \brief S5FHReceiveThread Constructs a new Receivethread
   * \param period The relative period after which the thread is
   *               cyclically woken up.
   * \param device handle of the serial device
   * \param received_callback function to call uppon finished packet
   */
  S5FHReceiveThread(const TimeSpan& period, Serial* device,
                    ReceivedPacketCallback const & received_callback);

  //! Default DTOR
  virtual ~S5FHReceiveThread() {}

  //! run method of the thread, executes the main program
  virtual void run();

  //! return the count of received packets
  unsigned int receivedPacketCount() { return m_packets_received; }

private:

  //! pointer to serial device object
  Serial* m_serial_device;

  //! enum for receive packet state machine states
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
  uint16_t m_length;

  //! length of received serial data
  std::vector<u_int8_t> m_data;

  //! pointer to array builder object for packet receive
  icl_comm::ArrayBuilder m_ab;

  //! packets counter
  unsigned int m_packets_received;

  //! state machine processing received data
  bool receiveData();

  //! function callback for received packages
  ReceivedPacketCallback m_received_callback;

};

}

#endif
