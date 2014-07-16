// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Lars Pfotzer
 * \date    2014-02-17
 * \date    2014-07-16
 *
 * This file contains the FeedBackpollingthread.
 * The hardware itself will not send data all the time, only once in response
 * to each packet sent. As the desired behaviour is to get constant position feedback
 * of the fingers we trigger a controllerfeedback periodically to receive continious data.
 * The feedback polling thread is implemented in this file.
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

// forward declaration as the fingermanager already uses this class
class S5FHFingerManager;

/*!
 * \brief Thread for periodically requesting feedback messages from the SCHUNK five finger hand.
 */
class S5FHFeedbackPollingThread : public PeriodicThread
{
public:
  /*!
   * \brief S5FHFeedbackPollingThread constructs a new thread to poll the feedback of all fingers periodically
   * \param period timespan after which the thread should be woken up
   * \param finger_manager reference to the fingermanager which functions are used to do the polling
   */
  S5FHFeedbackPollingThread(const TimeSpan& period, S5FHFingerManager* finger_manager);

  //! default DTOR
  virtual ~S5FHFeedbackPollingThread() {}

  //! run method of the thread
  virtual void run();

private:

  //! pointer to SCHUNK five finger hand fingermanager object
  S5FHFingerManager* m_finger_manager;

};

}

#endif
