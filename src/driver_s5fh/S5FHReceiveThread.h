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

using icl_core::TimeSpan;
using icl_core::thread::PeriodicThread;
using icl_comm::serial::Serial;

namespace driver_s5fh {

/*! Thread for periodically receiving messages from the serial device.
 */
class S5FHReceiveThread : public PeriodicThread
{
public:
  S5FHReceiveThread(const TimeSpan& period, Serial* serial_device)
    : PeriodicThread("S5FHReceiveThread", period),
      m_serial_device(serial_device)
  { }

  virtual ~S5FHReceiveThread() {}

  virtual void run()
  {
    // Wait for the first period so that the timing is in sync.
    waitPeriod();

    // TODO: Do things
  }

private:

  // pointer to serial interface object
  Serial *m_serial_device;

};

}

#endif
