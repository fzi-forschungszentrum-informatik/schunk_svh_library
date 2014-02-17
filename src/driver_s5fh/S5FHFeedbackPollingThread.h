// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Lars Pfotzer
 * \date    2014-02-17
 *
 */
//----------------------------------------------------------------------
#ifndef DRIVER_S5FH_S5FH_FEEDBACK_POLLING_THREAD_H_INCLUDED
#define DRIVER_S5FH_S5FH_FEEDBACK_POLLING_THREAD_H_INCLUDED

#include <icl_core/TimeSpan.h>
#include <icl_core_thread/PeriodicThread.h>
#include <driver_s5fh/S5FHController.h>

using icl_core::TimeSpan;
using icl_core::thread::PeriodicThread;

namespace driver_s5fh {

class S5FHFingerManager;

/*! Thread for periodically requesting feedback messages from the SCHUNK five finger hand.
 */
class S5FHFeedbackPollingThread : public PeriodicThread
{
public:
  S5FHFeedbackPollingThread(const TimeSpan& period, S5FHFingerManager* finger_manager);

  virtual ~S5FHFeedbackPollingThread() {}

  //! run method of the thread
  virtual void run();

private:

  //! pointer to SCHUNK five finger hand controller object
  S5FHFingerManager* m_finger_manager;

};

}

#endif
