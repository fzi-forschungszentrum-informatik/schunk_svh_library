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
#include <driver_s5fh/Logging.h>
#include <driver_s5fh/S5FHFingerManager.h>

namespace driver_s5fh {

S5FHFeedbackPollingThread::S5FHFeedbackPollingThread(const TimeSpan& period, S5FHFingerManager* finger_manager)
  : PeriodicThread("S5FHReceiveThread", period),
    m_finger_manager(finger_manager)
{
}

void S5FHFeedbackPollingThread::run()
{
  while (execute())
  {
    if (m_finger_manager != NULL)
    {
      if (m_finger_manager->isConnected())
      {
        for (size_t i = 0; i < eS5FH_DIMENSION; ++i)
        {
          S5FHCHANNEL channel = static_cast<S5FHCHANNEL>(i);
          if (m_finger_manager->isHomed(channel) && m_finger_manager->isEnabled(channel))
          {
            m_finger_manager->requestControllerFeedback(channel);
          }
        }
      }
      else
      {
        LOGGING_WARNING_C(DriverS5FH, S5FHFeedbackPollingThread, "SCHUNK five finger hand is not connected!" << endl);
      }
    }
    else
    {
      LOGGING_WARNING_C(DriverS5FH, S5FHFeedbackPollingThread, "Pointer to FingerManager is NULL!" << endl);
    }

    // Wait for the thread period so that the timing is in sync.
    waitPeriod();
  }
}

}