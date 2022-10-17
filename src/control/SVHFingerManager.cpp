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
 * \author  Georg Heppner
 * \date    2014-01-30
 * \date    2014-08-20
 *
 * This file contains the Finger Manager
 * that is managing the Schunk five finger hand on a high level.
 * The fingermanager is responsible to filter all calls and only make
 * allowed calls to the controller. The fingermanager is responsible
 * for storing any kind of configuration (like current controller settings).
 * The fingermanager is also responsible to poll the controller for continious data (if wanted)
 *
 */
//----------------------------------------------------------------------
#include <schunk_svh_library/Logger.h>
#include <schunk_svh_library/control/SVHFingerManager.h>

#include <chrono>
#include <cmath>
#include <functional>
#include <memory>
#include <thread>


namespace driver_svh {

SVHFingerManager::SVHFingerManager(const std::vector<bool>& disable_mask,
                                   const uint32_t& reset_timeout)
  : m_controller(new SVHController())
  , m_feedback_thread()
  , m_connected(false)
  , m_connection_feedback_given(false)
  , m_homing_timeout(10)
  , m_ticks2rad(0)
  , m_position_min(SVH_DIMENSION, 0)
  , m_position_max(SVH_DIMENSION, 0)
  , m_position_home(SVH_DIMENSION, 0)
  , m_is_homed(SVH_DIMENSION, false)
  , m_is_switched_off(SVH_DIMENSION, false)
  , m_diagnostic_encoder_state(SVH_DIMENSION, false)
  , m_diagnostic_current_state(SVH_DIMENSION, false)
  , m_diagnostic_current_maximum(SVH_DIMENSION, 0)
  , m_diagnostic_current_minimum(SVH_DIMENSION, 0)
  , m_diagnostic_position_maximum(SVH_DIMENSION, 0)
  , m_diagnostic_position_minimum(SVH_DIMENSION, 0)
  , m_diagnostic_deadlock(SVH_DIMENSION, 0)
  , m_reset_speed_factor(0.2)
  , m_reset_timeout(reset_timeout)
  , m_current_settings(SVH_DIMENSION)
  , m_current_settings_given(SVH_DIMENSION, false)
  , m_position_settings(SVH_DIMENSION)
  , m_position_settings_given(SVH_DIMENSION, false)
  , m_home_settings(SVH_DIMENSION)
  , m_serial_device("/dev/ttyUSB0")
{
  // load home position default parameters
  setDefaultHomeSettings();

  // set default reset order of all channels
  m_reset_order.resize(SVH_DIMENSION);
  m_reset_order[0] = SVH_INDEX_FINGER_PROXIMAL;
  m_reset_order[1] = SVH_MIDDLE_FINGER_PROXIMAL;
  m_reset_order[2] = SVH_THUMB_OPPOSITION;
  m_reset_order[3] = SVH_THUMB_FLEXION;
  m_reset_order[4] = SVH_FINGER_SPREAD;
  m_reset_order[5] = SVH_MIDDLE_FINGER_DISTAL;
  m_reset_order[6] = SVH_INDEX_FINGER_DISTAL;
  m_reset_order[7] = SVH_RING_FINGER;
  m_reset_order[8] = SVH_PINKY;

  for (size_t i = 0; i < SVH_DIMENSION; ++i)
  {
    m_is_switched_off[i] = disable_mask[i];
    if (m_is_switched_off[i])
    {
      SVH_LOG_INFO_STREAM("SVHFingerManager",
                          "Joint: "
                            << m_controller->m_channel_description[i]
                            << " was disabled as per user request. It will not do anything!");
    }
  }

  m_diagnostic_encoder_state.resize(SVH_DIMENSION, false);
  m_diagnostic_current_state.resize(SVH_DIMENSION, false);
  m_diagnostic_current_maximum.resize(SVH_DIMENSION, 0.0);
  m_diagnostic_current_minimum.resize(SVH_DIMENSION, 0.0);
  m_diagnostic_position_maximum.resize(SVH_DIMENSION, 0.0);
  m_diagnostic_position_minimum.resize(SVH_DIMENSION, 0.0);
  m_diagnostic_deadlock.resize(SVH_DIMENSION, 0.0);

  m_firmware_info.version_major = 0;
  m_firmware_info.version_minor = 0;
}

SVHFingerManager::~SVHFingerManager()
{
  if (m_connected)
  {
    disconnect();
  }

  if (m_controller != NULL)
  {
    delete m_controller;
    m_controller = NULL;
  }
}

bool SVHFingerManager::connect(const std::string& dev_name, const unsigned int& retry_count)
{
  SVH_LOG_DEBUG_STREAM("SVHFingerManager",
                       "Finger manager is trying to connect to the Hardware...");

  // Save device handle for next use
  m_serial_device = dev_name;


  if (m_connected)
  {
    disconnect();
  }

  if (m_controller != NULL)
  {
    if (m_controller->connect(dev_name))
    {
      unsigned int num_retries = retry_count;
      do
      {
        // Reset the package counts (in case a previous attempt was made)
        m_controller->resetPackageCounts();

        // load default position settings before the fingers are resetted
        std::vector<SVHPositionSettings> position_settings = getDefaultPositionSettings(true);

        // load default current settings
        std::vector<SVHCurrentSettings> current_settings = getDefaultCurrentSettings();

        m_controller->disableChannel(SVH_ALL);

        // initialize all channels
        for (size_t i = 0; i < SVH_DIMENSION; ++i)
        {
          // request controller feedback to have a valid starting point
          m_controller->requestControllerFeedback(static_cast<SVHChannel>(i));

          // Actually set the new position settings
          m_controller->setPositionSettings(static_cast<SVHChannel>(i), position_settings[i]);

          // set current settings
          m_controller->setCurrentSettings(static_cast<SVHChannel>(i), current_settings[i]);
        }

        // check for correct response from hardware controller
        auto start_time             = std::chrono::high_resolution_clock::now();
        bool timeout                = false;
        unsigned int received_count = 0;
        unsigned int send_count     = 0;
        while (!timeout && !m_connected)
        {
          send_count     = m_controller->getSentPackageCount();
          received_count = m_controller->getReceivedPackageCount();
          if (send_count == received_count)
          {
            m_connected = true;
            SVH_LOG_INFO_STREAM("SVHFingerManager",
                                "Successfully established connection to SCHUNK five finger hand."
                                  << "Send packages = " << send_count
                                  << ", received packages = " << received_count);
          }
          SVH_LOG_DEBUG_STREAM("SVHFingerManager",
                               "Try to connect to SCHUNK five finger hand: Send packages = "
                                 << send_count << ", received packages = " << received_count);

          // check for timeout
          if ((std::chrono::high_resolution_clock::now() - start_time) > m_reset_timeout)
          {
            timeout = true;
            SVH_LOG_ERROR_STREAM("SVHFingerManager",
                                 "Connection timeout! Could not connect to SCHUNK five finger hand."
                                   << "Send packages = " << send_count
                                   << ", received packages = " << received_count);
          }
          std::this_thread::sleep_for(std::chrono::microseconds(50000));
        }

        // Try again, but ONLY if we at least got one package back, otherwise its futil
        if (!m_connected)
        {
          if (received_count > 0 && num_retries >= 0)
          {
            num_retries--;
            SVH_LOG_ERROR_STREAM("SVHFingerManager",
                                 "Connection Failed! Send packages = "
                                   << send_count << ", received packages = " << received_count
                                   << ". Retrying, count: " << num_retries);
          }
          else
          {
            num_retries = 0;
            SVH_LOG_ERROR_STREAM("SVHFingerManager",
                                 "Connection Failed! Send packages = "
                                   << send_count << ", received packages = " << received_count
                                   << ". Not Retrying anymore.");
          }
        }
        // Keep trying to reconnect several times because the brainbox often makes problems
      } while (!m_connected && num_retries > 0);


      if (!m_connected && num_retries <= 0)
      {
        SVH_LOG_ERROR_STREAM("SVHFingerManager",
                             "A Stable connection could NOT be made, however some packages where "
                             "received. Please check the hardware!");
      }


      if (m_connected)
      {
        // Request firmware information once at the beginning, it will print out on the console
        m_controller->requestFirmwareInfo();

        // initialize feedback polling thread
        if (m_feedback_thread.joinable()) // clean reset
        {
          m_poll_feedback = false;
          m_feedback_thread.join();
        }
        m_poll_feedback   = true;
        m_feedback_thread = std::thread(&SVHFingerManager::pollFeedback, this);
        SVH_LOG_DEBUG_STREAM("SVHFingerManager",
                             "Finger manager is starting the fedback polling thread");
      }
      else
      {
        // connection open but not stable: close serial port for better reconnect later
        m_controller->disconnect();
      }
    }
    else
    {
      SVH_LOG_ERROR_STREAM("SVHFingerManager", "Connection FAILED! Device could NOT be opened");
    }
  }

  return m_connected;
}

void SVHFingerManager::disconnect()
{
  SVH_LOG_DEBUG_STREAM("SVHFingerManager",
                       "Finger manager is trying to discoconnect to the Hardware...");
  m_connected                 = false;
  m_connection_feedback_given = false;

  // Disable Polling
  m_poll_feedback = false;
  if (m_feedback_thread.joinable())
  {
    m_feedback_thread.join();
    SVH_LOG_DEBUG_STREAM("SVHFingerManager", "Feedback thread terminated");
  }

  // Tell the Controller to terminate the rest
  if (m_controller != NULL)
  {
    m_controller->disconnect();
  }
}

//! reset function for a single finger
bool SVHFingerManager::resetChannel(const SVHChannel& channel)
{
  if (m_connected)
  {
    // reset all channels
    if (channel == SVH_ALL)
    {
      bool reset_all_success = true;
      for (size_t i = 0; i < SVH_DIMENSION; ++i)
      {
        // try three times to reset each finger
        size_t max_reset_counter = 3;
        bool reset_success       = false;
        while (!reset_success && max_reset_counter > 0)
        {
          SVHChannel channel = static_cast<SVHChannel>(m_reset_order[i]);
          reset_success      = resetChannel(channel);
          max_reset_counter--;
        }

        SVH_LOG_DEBUG_STREAM(
          "resetChannel", "Channel " << m_reset_order[i] << " reset success = " << reset_success);

        // set all reset flag
        reset_all_success = reset_all_success && reset_success;
      }

      return reset_all_success;
    }
    else if (channel > SVH_ALL && SVH_ALL < SVH_DIMENSION)
    {
      m_diagnostic_encoder_state[channel] = false;
      m_diagnostic_current_state[channel] = false;

      SVH_LOG_DEBUG_STREAM("SVHFingerManager", "Start homing channel " << channel);

      if (!m_is_switched_off[channel])
      {
        SVH_LOG_DEBUG_STREAM("SVHFingerManager",
                             "Setting reset position values for controller of channel " << channel);

        m_controller->setPositionSettings(channel, getDefaultPositionSettings(true)[channel]);

        // reset homed flag
        m_is_homed[channel] = false;

        // read default home settings for channel
        SVHHomeSettings home = m_home_settings[channel];

        SVHPositionSettings pos_set;
        SVHCurrentSettings cur_set;
        m_controller->getPositionSettings(channel, pos_set);
        m_controller->getCurrentSettings(channel, cur_set);

        // find home position
        int32_t position = 0;

        if (home.direction > 0)
        {
          position = static_cast<int32_t>(pos_set.wmx);
        }
        else
        {
          position = static_cast<int32_t>(pos_set.wmn);
        }

        SVH_LOG_INFO_STREAM("SVHFingerManager",
                            "Driving channel "
                              << channel << " to hardstop. Detection thresholds: Current MIN: "
                              << home.reset_current_factor * cur_set.wmn
                              << "mA MAX: " << home.reset_current_factor * cur_set.wmx << "mA");

        m_controller->setControllerTarget(channel, position);
        m_controller->enableChannel(channel);

        SVHControllerFeedback control_feedback_previous;
        SVHControllerFeedback control_feedback;

        // initialize timeout
        auto start_time     = std::chrono::high_resolution_clock::now();
        auto start_time_log = std::chrono::high_resolution_clock::now();
        // Debug helper to just notify about fresh stales
        bool stale_notification_sent = false;

        for (size_t hit_count = 0; hit_count < 10;)
        {
          m_controller->setControllerTarget(channel, position);
          // m_controller->requestControllerFeedback(channel);
          m_controller->getControllerFeedback(channel, control_feedback);
          // Timeout while no encoder ticks changed

          // Quite extensive Current output!
          if (std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now() - start_time_log) >
              std::chrono::milliseconds(1000))
          {
            SVH_LOG_INFO_STREAM("SVHFingerManager",
                                "Resetting Channel "
                                  << channel << ":" << m_controller->m_channel_description[channel]
                                  << " current: " << control_feedback.current << " mA");
            start_time_log = std::chrono::high_resolution_clock::now();
          }

          double threshold = 80;
          // have a look for deadlocks
          if (home.direction == +1)
          {
            double delta =
              control_feedback.current -
              m_diagnostic_current_maximum[channel]; // without deadlocks delta should be positiv
            if (delta <= -threshold)
            {
              if (std::abs(delta) > m_diagnostic_deadlock[channel])
              {
                m_diagnostic_deadlock[channel] = std::abs(delta);
              }
            }
          }
          else
          {
            double delta = control_feedback.current - m_diagnostic_current_minimum[channel];
            if (delta >= threshold)
            {
              if (std::abs(delta) > m_diagnostic_deadlock[channel])
              {
                m_diagnostic_deadlock[channel] = std::abs(delta);
              }
            }
          }

          // save the maximal/minimal current of the motor
          if (control_feedback.current > m_diagnostic_current_maximum[channel])
          {
            m_diagnostic_current_maximum[channel] = control_feedback.current;
          }
          else
          {
            if (control_feedback.current < m_diagnostic_current_minimum[channel])
            {
              m_diagnostic_current_minimum[channel] = control_feedback.current;
            }
          }

          if ((home.reset_current_factor * cur_set.wmn >= control_feedback.current) ||
              (control_feedback.current >= home.reset_current_factor * cur_set.wmx))
          {
            m_diagnostic_current_state[channel] =
              true; // when in maximum the current controller is ok

            hit_count++;
            SVH_LOG_DEBUG_STREAM("SVHFingerManager",
                                 "Resetting Channel "
                                   << channel << ":" << m_controller->m_channel_description[channel]
                                   << " Hit Count increased: " << hit_count);
          }
          else if (hit_count > 0)
          {
            hit_count--;
            SVH_LOG_DEBUG_STREAM("SVHFingerManager",
                                 "Resetting Channel "
                                   << channel << ":" << m_controller->m_channel_description[channel]
                                   << " Hit Count Decreased: " << hit_count);
          }

          // check for time out: Abort, if position does not change after homing timeout.
          if ((std::chrono::high_resolution_clock::now() - start_time) > m_homing_timeout)
          {
            m_controller->disableChannel(SVH_ALL);
            SVH_LOG_ERROR_STREAM("SVHFingerManager",
                                 "Timeout: Aborted finding home position for channel " << channel);
            // Timeout could mean serious hardware issues or just plain wrong settings
            return false;
          }

          // reset time if position changes
          if (control_feedback.position != control_feedback_previous.position)
          {
            m_diagnostic_encoder_state[channel] = true;
            // save the maximal/minimal position the channel can reach
            if (control_feedback.position > m_diagnostic_position_maximum[channel])
              m_diagnostic_position_maximum[channel] = control_feedback.position;
            else if (control_feedback.position < m_diagnostic_position_minimum[channel])
              m_diagnostic_position_minimum[channel] = control_feedback.position;

            start_time = std::chrono::high_resolution_clock::now();
            if (stale_notification_sent)
            {
              SVH_LOG_DEBUG_STREAM("SVHFingerManager",
                                   "Resetting Channel "
                                     << channel << ":"
                                     << m_controller->m_channel_description[channel]
                                     << " Stale resolved, continuing detection");
              stale_notification_sent = false;
            }
          }
          else
          {
            if (!stale_notification_sent)
            {
              SVH_LOG_DEBUG_STREAM("SVHFingerManager",
                                   "Resetting Channel "
                                     << channel << ":"
                                     << m_controller->m_channel_description[channel]
                                     << " Stale detected. Starting Timeout");
              stale_notification_sent = true;
            }
          }

          // save previous control feedback
          control_feedback_previous = control_feedback;
          // std::this_thread::sleep_for(std::chrono::microseconds(8000));
        }
        // give the last info with highes channel current value
        SVH_LOG_INFO_STREAM("SVHFingerManager",
                            "Resetting Channel "
                              << channel << ":" << m_controller->m_channel_description[channel]
                              << " current: " << control_feedback.current << " mA");

        SVH_LOG_DEBUG_STREAM("SVHFingerManager", "Hit counter of " << channel << " reached.");


        // set reference values
        m_position_min[channel] = static_cast<int32_t>(
          control_feedback.position + std::min(home.minimum_offset, home.maximum_offset));
        m_position_max[channel] = static_cast<int32_t>(
          control_feedback.position + std::max(home.minimum_offset, home.maximum_offset));
        m_position_home[channel] =
          static_cast<int32_t>(control_feedback.position + home.direction * home.idle_position);
        SVH_LOG_DEBUG_STREAM("SVHFingerManager",
                             "Setting soft stops for Channel "
                               << channel << " min pos = " << m_position_min[channel]
                               << " max pos = " << m_position_max[channel]
                               << " home pos = " << m_position_home[channel]);

        // position will now be reached to release the motor and go into soft stops
        position = m_position_home[channel];

        // go to idle position
        // use the declared start_time variable for the homing timeout
        start_time = std::chrono::high_resolution_clock::now();
        while (true)
        {
          m_controller->setControllerTarget(channel, position);
          // m_controller->requestControllerFeedback(channel);
          m_controller->getControllerFeedback(channel, control_feedback);

          SVH_LOG_DEBUG_STREAM("SVHFingerManager",
                               "Homing Channel "
                                 << channel << ":" << m_controller->m_channel_description[channel]
                                 << " current: " << control_feedback.current
                                 << " mA, position ticks: " << control_feedback.position);

          if (abs(position - control_feedback.position) < 1000)
          {
            m_is_homed[channel] = true;
            break;
          }

          // if the finger hasn't reached the home position after m_homing_timeout there is an
          // hardware error
          if ((std::chrono::high_resolution_clock::now() - start_time) > m_homing_timeout)
          {
            m_is_homed[channel] = false;
            SVH_LOG_ERROR_STREAM("SVHFingerManager",
                                 "Channel " << channel << " home position is not reachable after "
                                            << m_homing_timeout.count()
                                            << "s! There could be an hardware error!");
            break;
          }
        }

        m_controller->disableChannel(SVH_ALL);
        // std::this_thread::sleep_for(std::chrono::microseconds(8000));
        SVH_LOG_DEBUG_STREAM("SVHFingerManager",
                             "Restoring default position values for controller of channel "
                               << channel);
        m_controller->setPositionSettings(channel, getDefaultPositionSettings(false)[channel]);
      }
      else
      {
        SVH_LOG_INFO_STREAM("SVHFingerManager",
                            "Channel " << channel
                                       << "switched of by user, homing is set to finished");
        m_is_homed[channel] = true;
      }

      // Check if this reset has trigger the reset of all the Fingers
      // bool reset_all_success = true;
      // for (size_t i = 0; i < SVH_DIMENSION; ++i)
      // {
      //   reset_all_success == reset_all_success&& m_is_homed[channel];
      // }

      SVH_LOG_INFO_STREAM("SVHFingerManager", "Successfully homed channel " << channel);

      return true;
    }
    else
    {
      SVH_LOG_ERROR_STREAM("SVHFingerManager", "Channel " << channel << " is out of bounds!");
      return false;
    }
  }
  else
  {
    SVH_LOG_ERROR_STREAM("SVHFingerManager",
                         "Could not reset channel "
                           << channel << ": No connection to SCHUNK five finger hand!");
    return false;
  }
}

bool SVHFingerManager::getDiagnosticStatus(const SVHChannel& channel,
                                           struct DiagnosticState& diagnostic_status)
{
  if (channel >= 0 && channel < SVH_DIMENSION)
  {
    diagnostic_status.diagnostic_encoder_state    = m_diagnostic_encoder_state[channel];
    diagnostic_status.diagnostic_motor_state      = m_diagnostic_current_state[channel];
    diagnostic_status.diagnostic_current_maximum  = m_diagnostic_current_maximum[channel];
    diagnostic_status.diagnostic_current_minimum  = m_diagnostic_current_minimum[channel];
    diagnostic_status.diagnostic_position_maximum = m_diagnostic_position_maximum[channel];
    diagnostic_status.diagnostic_position_minimum = m_diagnostic_position_minimum[channel];
    diagnostic_status.diagnostic_deadlock         = m_diagnostic_deadlock[channel];
    return true;
  }
  else
  {
    SVH_LOG_ERROR_STREAM("SVHFingerManager",
                         "Could not get diagnostic status for unknown/unsupported channel "
                           << channel);
    return false;
  }
}


// enables controller of channel
bool SVHFingerManager::enableChannel(const SVHChannel& channel)
{
  if (isConnected() && isHomed(channel))
  {
    if (channel == SVH_ALL)
    {
      for (size_t i = 0; i < SVH_DIMENSION; ++i)
      {
        // Just for safety, enable channels in the same order as we have resetted them (otherwise
        // developers might geht confused)
        SVHChannel real_channel = static_cast<SVHChannel>(m_reset_order[i]);
        if (!m_is_switched_off[real_channel])
        {
          // recursion to get the other updates corresponing with activation of a channel
          enableChannel(real_channel);
        }
      }
    }
    else if (channel > SVH_ALL && SVH_ALL < SVH_DIMENSION)
    {
      // Note: This part is another one of these places where the names can lead to confusion. I am
      // sorry about that Switched off is a logical term. The user has chosen NOT to use this
      // channel because of hardware trouble. To enable a smooth driver behaviour all replys
      // regarding these channels will be answered in the most positive way the caller could expect.
      // Enabled refers to the actual enabled state of the hardware controller loops that drive the
      // motors. As the user has chosen not to use certain channels we explicitly do NOT enable
      // these but tell a calling driver that we did
      if (!m_is_switched_off[channel])
      {
        m_controller->enableChannel(channel);
      }
    }
    return true;
  }
  return false;
}

void SVHFingerManager::disableChannel(const SVHChannel& channel)
{
  if (channel == SVH_ALL)
  {
    for (size_t i = 0; i < SVH_DIMENSION; ++i)
    {
      disableChannel(static_cast<SVHChannel>(i));
    }
  }
  else
  {
    if (!m_is_switched_off[channel])
    {
      m_controller->disableChannel(channel);
    }

    bool all_disabled = true;
    for (size_t i = 0; i < SVH_DIMENSION; ++i)
    {
      // Again only check channels that are not switched off. Switched off channels will always
      // answer that they are enabled
      all_disabled =
        all_disabled && (m_is_switched_off[channel] || !isEnabled(static_cast<SVHChannel>(i)));
    }
  }
}

bool SVHFingerManager::requestControllerFeedback(const SVHChannel& channel)
{
  if (isConnected())
  {
    m_controller->requestControllerFeedback(channel);
    return true;
  }

  SVH_LOG_WARN_STREAM("SVHFingerManager",
                      "Feedback for channel "
                        << channel << " could not be requested. FM is not connected to HW.");
  return false;
}

// returns actual position value for given channel
bool SVHFingerManager::getPosition(const SVHChannel& channel, double& position)
{
  SVHControllerFeedback controller_feedback;
  if ((channel >= 0 && channel < SVH_DIMENSION) && isHomed(channel) &&
      m_controller->getControllerFeedback(channel, controller_feedback))
  {
    // Switched off channels will always remain at zero position as the tics we get back migh be
    // total gibberish
    if (m_is_switched_off[channel])
    {
      position = 0.0;
      return true;
    }

    // int32_t cleared_position_ticks = controller_feedback.position;
    position = convertTicks2Rad(channel, controller_feedback.position);

    // Safety overwrite: If controller drives to a negative position (should not happen but might in
    // case the soft stops are placed badly) we cannot get out because inputs smaller than 0 will be
    // ignored
    if (position < 0)
    {
      position = 0.0;
    }

    // DISABLED as the output was realy spamming everything else :)
    // SVH_LOG_DEBUG_STREAM("SVHFingerManager", "Channel " << channel << ": position_ticks = " <<
    // controller_feedback.position
    //                << " | cleared_position_ticks = " << cleared_position_ticks << " | position
    //                rad = " << position);
    return true;
  }
  else
  {
    SVH_LOG_WARN_STREAM("SVHFingerManager", "Could not get postion for channel " << channel);
    return false;
  }
}

// returns actual current value for given channel
bool SVHFingerManager::getCurrent(const SVHChannel& channel, double& current)
{
  SVHControllerFeedback controller_feedback;
  if ((channel >= 0 && channel < SVH_DIMENSION) && isHomed(channel) &&
      m_controller->getControllerFeedback(channel, controller_feedback))
  {
    current = controller_feedback.current;
    return true;
  }
  else
  {
    SVH_LOG_WARN_STREAM("SVHFingerManager", "Could not get current for channel " << channel);
    return false;
  }
}

// set all target positions at once
bool SVHFingerManager::setAllTargetPositions(const std::vector<double>& positions)
{
  if (isConnected())
  {
    // check size of position vector
    if (positions.size() == SVH_DIMENSION)
    {
      // create target positions vector
      std::vector<int32_t> target_positions(SVH_DIMENSION, 0);

      bool reject_command = false;
      for (size_t i = 0; i < SVH_DIMENSION; ++i)
      {
        SVHChannel channel = static_cast<SVHChannel>(i);

        // enable all homed and disabled channels.. except its switched of
        if (!m_is_switched_off[channel] && isHomed(channel) && !isEnabled(channel))
        {
          enableChannel(channel);
        }

        // convert all channels to ticks
        target_positions[channel] = convertRad2Ticks(channel, positions[channel]);

        // check for out of bounds (except the switched off channels)
        if (!m_is_switched_off[channel] && !isInsideBounds(channel, target_positions[channel]))
        {
          reject_command = true;
        }
      }

      // send target position vector to controller and SCHUNK hand
      if (!reject_command)
      {
        m_controller->setControllerTargetAllChannels(target_positions);
        return true;
      }
      else
      {
        SVH_LOG_WARN_STREAM(
          "SVHFingerManager",
          "Could not set target position vector: At least one channel is out of bounds!");
        return false;
      }
    }
    else
    {
      SVH_LOG_WARN_STREAM("SVHFingerManager",
                          "Size of target position vector wrong: size = "
                            << positions.size() << " expected size = " << (int)SVH_DIMENSION);
      return false;
    }
  }
  else
  {
    if (!m_connection_feedback_given)
    {
      SVH_LOG_ERROR_STREAM(
        "SVHFingerManager",
        "Could not set target position vector: No connection to SCHUNK five finger hand!");
      m_connection_feedback_given = true;
    }
    return false;
  }
}

bool SVHFingerManager::setTargetPosition(const SVHChannel& channel, double position, double current)
{
  if (isConnected())
  {
    if (channel >= 0 && channel < SVH_DIMENSION)
    {
      if (m_is_switched_off[channel])
      {
        // Switched off channels  behave transparent so we return a true value while we ignore the
        // input
        SVH_LOG_DEBUG_STREAM("SVHFingerManager",
                             "Target position for channel "
                               << channel << " was ignored as it is switched off by the user");
        return true;
      }


      if (isHomed(channel))
      {
        int32_t target_position = convertRad2Ticks(channel, position);

        // Disabled as the output will spam everything
        // SVH_LOG_DEBUG_STREAM("SVHFingerManager", "Target position for channel " << channel << " =
        // " << target_position);

        // check for bounds
        if (isInsideBounds(channel, target_position))
        {
          if (!isEnabled(channel))
          {
            enableChannel(channel);
          }

          m_controller->setControllerTarget(channel, target_position);
          return true;
        }
        else
        {
          SVH_LOG_ERROR_STREAM("SVHFingerManager",
                               "Target position for channel " << channel << " out of bounds!");
          return false;
        }
      }
      else
      {
        SVH_LOG_ERROR_STREAM("SVHFingerManager",
                             "Could not set target position for channel " << channel
                                                                          << ": Reset first!");
        return false;
      }
    }
    else
    {
      SVH_LOG_ERROR_STREAM("SVHFingerManager",
                           "Could not set target position for channel " << channel
                                                                        << ": Illegal Channel");
      return false;
    }
  }
  else
  {
    // Give the Warning about no Connection exactly once! Otherwise this will immediately spam the
    // log
    if (!m_connection_feedback_given)
    {
      SVH_LOG_ERROR_STREAM("SVHFingerManager",
                           "Could not set target position for channel "
                             << channel << ": No connection to SCHUNK five finger hand!");
      m_connection_feedback_given = true;
    }
    return false;
  }
}

// return enable flag
bool SVHFingerManager::isEnabled(const SVHChannel& channel)
{
  if (channel == SVH_ALL)
  {
    bool all_enabled = true;
    for (size_t i = 0; i < SVH_DIMENSION; ++i)
    {
      all_enabled = all_enabled && isEnabled(static_cast<SVHChannel>(i));
      // disabled for now, to noisy
      //      if (!isEnabled(static_cast<SVHChannel>(i)))
      //      {
      //        SVH_LOG_WARN_STREAM("SVHFingerManager", "All finger enabled check failed: Channel: "
      //        << channel << " : " << SVHController::m_channel_description[i] << " is not
      //        enabled");
      //      }
    }

    return all_enabled;
  }
  else if (channel >= 0 && channel < SVH_DIMENSION)
  {
    // Switched off Channels will aways be reported as enabled to simulate everything is fine.
    // Others need to ask the controller if the channel is realy switched on Note: i can see that
    // based on the names this might lead to a little confusion... sorry about that but there are
    // only limited number of words for not active ;) enabled refers to the actual state of the
    // position and current controllers. So enabled means enabled on a hardware level. Switched off
    // is a logical decission in this case. The user has specified this particular channel not to be
    // used (due to hardware issues) and therefore the driver (aka the finger manager) will act AS
    // IF the channel was enabled but is in fact switched off by the user. If you have a better
    // variable name or a better idea how to handle that you are welcome to change it. (GH
    // 2014-05-26)
    return (m_is_switched_off[channel] || m_controller->isEnabled(channel));
  }
  else
  {
    SVH_LOG_ERROR_STREAM("SVHFingerManager",
                         "isEnabled was requested for UNKNOWN Channel: " << channel);
    return false;
  }
}

bool SVHFingerManager::isHomed(const SVHChannel& channel)
{
  if (channel == SVH_ALL)
  {
    bool all_homed = true;
    for (size_t i = 0; i < SVH_DIMENSION; ++i)
    {
      all_homed = all_homed && isHomed(static_cast<SVHChannel>(i));
      if (!isHomed(static_cast<SVHChannel>(i)))
      {
        SVH_LOG_WARN_STREAM("SVHFingerManager",
                            "All finger homed check failed: Channel: "
                              << i << " : " << SVHController::m_channel_description[i]
                              << " is not homed");
      }
    }

    return all_homed;
  }
  else if (channel >= 0 && channel < SVH_DIMENSION)
  {
    // Channels that are switched off will always be reported as homed to simulate everything is
    // fine. Others have to check
    return (m_is_switched_off[channel] || m_is_homed[channel]);
  }
  else // should not happen but better be save than sorry
  {
    SVH_LOG_ERROR_STREAM("SVHFingerManager",
                         "isHomed was requested for UNKNOWN Channel: " << channel);
    return false;
  }
}

bool SVHFingerManager::getCurrentSettings(const SVHChannel& channel,
                                          SVHCurrentSettings& current_settings)
{
  if (channel >= 0 && channel < SVH_DIMENSION)
  {
    return m_controller->getCurrentSettings(channel, current_settings);
  }
  else
  {
    SVH_LOG_ERROR_STREAM("SVHFingerManager",
                         "Could not get current settings for unknown/unsupported channel "
                           << channel);
    return false;
  }
}

bool SVHFingerManager::getPositionSettings(const SVHChannel& channel,
                                           SVHPositionSettings& position_settings)
{
  if (channel >= 0 && channel < SVH_DIMENSION)
  {
    return m_controller->getPositionSettings(channel, position_settings);
  }
  else
  {
    SVH_LOG_ERROR_STREAM("SVHFingerManager",
                         "Could not get position settings for unknown/unsupported channel "
                           << channel);
    return false;
  }
}

bool SVHFingerManager::getHomeSettings(const SVHChannel& channel, SVHHomeSettings& home_settings)
{
  if (channel >= 0 && channel < SVH_DIMENSION)
  {
    home_settings = m_home_settings[channel];
    return true;
  }
  else
  {
    SVH_LOG_ERROR_STREAM("SVHFingerManager",
                         "Could not get home settings for unknown/unsupported channel " << channel);
    return false;
  }
}

bool SVHFingerManager::currentSettingsAreSafe(const SVHChannel& channel,
                                              const SVHCurrentSettings& current_settings)
{
  bool settings_are_safe = false;

  if (!isEnabled(SVH_ALL))
  {
    SVH_LOG_DEBUG_STREAM("SVHFingerManager", "Fingers are not all enabled -> no safety tests");
    // befor the fingers are homed no finger-data are valid
    return true;
  }

  if (current_settings.wmx <=
      m_max_current_percentage * std::max(m_diagnostic_current_maximum[channel],
                                          std::abs(m_diagnostic_position_minimum[channel])))
  {
    SVH_LOG_DEBUG_STREAM("SVHFingerManager", "Current settings are safe!");
    settings_are_safe = true;
  }
  else
  {
    SVH_LOG_WARN_STREAM("SVHFingerManager",
                        "Current value given: " << current_settings.wmx << " is not valid.");
    SVH_LOG_DEBUG_STREAM(
      "SVHFingerManager",
      " Please provide values between "
        << " 0 - "
        << m_max_current_percentage * std::max(m_diagnostic_current_maximum[channel],
                                               std::abs(m_diagnostic_position_minimum[channel]))
        << " [mA] or 0 - "
        << convertmAtoN(channel,
                        m_max_current_percentage *
                          std::max(m_diagnostic_current_maximum[channel],
                                   std::abs(m_diagnostic_position_minimum[channel])))
        << " [N]");
  }

  return settings_are_safe;
}

// overwrite current parameters
bool SVHFingerManager::setCurrentSettings(const SVHChannel& channel,
                                          const SVHCurrentSettings& current_settings)
{
  if (channel >= 0 && channel < SVH_DIMENSION)
  {
    // For now we will prefent current settings with more current than possible
    if (!currentSettingsAreSafe(channel, current_settings))
    {
      // SVH_LOG_ERROR_STREAM("SVHFingerManager", "WARNING!!! Current Controller Params for channel
      // " << channel << " are dangerous! THIS MIGHT DAMAGE YOUR HARDWARE!!!");
      SVH_LOG_ERROR_STREAM("SVHFingerManager",
                           "WARNING!!! Current Controller Params for channel "
                             << channel << " would be dangerous! Currents are limited!!!");
      return false;
    }

    // First of save the values
    m_current_settings[channel]       = current_settings;
    m_current_settings_given[channel] = true;

    // In case the Hardware is connected, update the values
    if (isConnected())
    {
      m_controller->setCurrentSettings(channel, current_settings);
    }
    return true;
  }
  else
  {
    SVH_LOG_ERROR_STREAM("SVHFingerManager",
                         "Could not set Current Controller Params for channel "
                           << channel << ": No such channel");
    return false;
  }
}

// overwrite position parameters
bool SVHFingerManager::setPositionSettings(const SVHChannel& channel,
                                           const SVHPositionSettings& position_settings)
{
  if (channel >= 0 && channel < SVH_DIMENSION)
  {
    // First of save the values
    m_position_settings[channel]       = position_settings;
    m_position_settings_given[channel] = true;

    // In case the Hardware is connected, update the values
    if (isConnected())
    {
      m_controller->setPositionSettings(channel, position_settings);
    }

    return true;
  }
  else
  {
    SVH_LOG_ERROR_STREAM("SVHFingerManager",
                         "Could not set Position Controller Params for channel "
                           << channel << ": No such channel");
    return false;
  }
}

// overwirte home settings
bool SVHFingerManager::setHomeSettings(const SVHChannel& channel,
                                       const driver_svh::SVHHomeSettings& home_settings)
{
  if (channel >= 0 && channel < SVH_DIMENSION)
  {
    // First of save the values
    m_home_settings[channel] = home_settings;
    SVH_LOG_DEBUG_STREAM("SVHFingerManager",
                         "Channel " << channel << " setting new homing settings : ");
    SVH_LOG_DEBUG_STREAM("SVHFingerManager",
                         "Direction " << home_settings.direction << " "
                                      << "Min offset " << home_settings.minimum_offset << " "
                                      << "Max offset " << home_settings.maximum_offset << " "
                                      << "idle pos " << home_settings.idle_position << " "
                                      << "Range Rad " << home_settings.range_rad << " "
                                      << "Reset Curr Factor " << home_settings.reset_current_factor
                                      << " ");

    // Update the conversion factor for this finger:
    float range_ticks =
      m_home_settings[channel].maximum_offset - m_home_settings[channel].minimum_offset;
    m_ticks2rad[channel] =
      m_home_settings[channel].range_rad / range_ticks * (-m_home_settings[channel].direction);

    return true;
  }
  else
  {
    SVH_LOG_ERROR_STREAM("SVHFingerManager",
                         "Could not set homing settings for channel " << channel
                                                                      << ": No such channel");
    return false;
  }
}

bool SVHFingerManager::resetDiagnosticData(const SVHChannel& channel)
{
  // reset all channels
  if (channel == SVH_ALL)
  {
    for (size_t i = 0; i <= SVH_DIMENSION; ++i)
    {
      m_diagnostic_encoder_state[i]    = false;
      m_diagnostic_current_state[i]    = false;
      m_diagnostic_current_maximum[i]  = 0.0;
      m_diagnostic_current_minimum[i]  = 0.0;
      m_diagnostic_position_maximum[i] = 0.0;
      m_diagnostic_position_minimum[i] = 0.0;
      m_diagnostic_deadlock[i]         = 0.0;
    }
    SVH_LOG_DEBUG_STREAM("SVHFingerManager",
                         "Diagnostic data for all channel reseted successfully");
    return true;
  }
  else
  {
    if (channel > 0 && channel <= SVH_DIMENSION)
    {
      m_diagnostic_encoder_state[channel]    = false;
      m_diagnostic_current_state[channel]    = false;
      m_diagnostic_current_maximum[channel]  = 0.0;
      m_diagnostic_current_minimum[channel]  = 0.0;
      m_diagnostic_position_maximum[channel] = 0.0;
      m_diagnostic_position_minimum[channel] = 0.0;
      SVH_LOG_DEBUG_STREAM("SVHFingerManager",
                           "Diagnostic data for channel " << channel << " reseted successfully");
      return true;
    }
    else
    {
      SVH_LOG_ERROR_STREAM("SVHFingerManager",
                           "Could not reset diagnostic data for channel " << channel
                                                                          << ": No such channel");
      return false;
    }
  }
}

void SVHFingerManager::setDefaultHomeSettings()
{
  // homing parameters are important for software end stops

  // All values are based on the hardware description for maximum tics and maximum allowable range
  // of movements direction, minimum offset, maximum offset, idle position, range in rad,
  // resetcurrent(factor)
  m_home_settings[SVH_THUMB_FLEXION] =
    SVHHomeSettings(+1, -175.0e3f, -5.0e3f, -15.0e3f, 0.97, 0.75); // thumb flexion
  // Conservative value
  // m_home_settings[eSVH_THUMB_OPPOSITION]       =  SVHHomeSettings(+1, -105.0e3f,  -5.0e3f,
  // -15.0e3f, 0.99, 0.75); // thumb opposition
  // Value using the complete movemment range
  m_home_settings[SVH_THUMB_OPPOSITION] =
    SVHHomeSettings(+1, -150.0e3f, -5.0e3f, -15.0e3f, 0.99, 0.75); // thumb opposition
  m_home_settings[SVH_INDEX_FINGER_DISTAL] =
    SVHHomeSettings(+1, -47.0e3f, -2.0e3f, -8.0e3f, 1.33, 0.75); // index finger distal joint
  m_home_settings[SVH_INDEX_FINGER_PROXIMAL] =
    SVHHomeSettings(-1, 2.0e3f, 42.0e3f, 8.0e3f, 0.8, 0.75); // index finger proximal joint
  m_home_settings[SVH_MIDDLE_FINGER_DISTAL] =
    SVHHomeSettings(+1, -47.0e3f, -2.0e3f, -8.0e3f, 1.33, 0.75); // middle finger distal joint
  m_home_settings[SVH_MIDDLE_FINGER_PROXIMAL] =
    SVHHomeSettings(-1, 2.0e3f, 42.0e3f, 8.0e3f, 0.8, 0.75); // middle finger proximal joint
  m_home_settings[SVH_RING_FINGER] =
    SVHHomeSettings(+1, -47.0e3f, -2.0e3f, -8.0e3f, 0.98, 0.75); // ring finger
  m_home_settings[SVH_PINKY] = SVHHomeSettings(+1, -47.0e3f, -2.0e3f, -8.0e3f, 0.98, 0.75); // pinky
  m_home_settings[SVH_FINGER_SPREAD] =
    SVHHomeSettings(+1, -47.0e3f, -2.0e3f, -25.0e3f, 0.58, 0.4); // finger spread

  m_ticks2rad.resize(SVH_DIMENSION, 0.0);
  for (size_t i = 0; i < SVH_DIMENSION; ++i)
  {
    float range_ticks = m_home_settings[i].maximum_offset - m_home_settings[i].minimum_offset;
    m_ticks2rad[i] = m_home_settings[i].range_rad / range_ticks * (-m_home_settings[i].direction);
  }
}


std::vector<SVHCurrentSettings> SVHFingerManager::getDefaultCurrentSettings()
{
  // BEWARE! Only change these values if you know what you are doing !! Setting wrong values could
  // damage the hardware!!!

  std::vector<SVHCurrentSettings> current_settings(SVH_DIMENSION);


  // curr min, Curr max,ky(error output scaling),dt(time base),imn (integral windup min), imx
  // (integral windup max), kp,ki,umn,umx (output limter) More accurate values used in the new param
  // files for SVH V1
  SVHCurrentSettings cur_set_thumb(
    -500.0f, 500.0f, 0.405f, 4e-6f, -25.0f, 25.0f, 0.6f, 10.0f, -255.0f, 255.0f);
  SVHCurrentSettings cur_set_thumb_opposition(
    -500.0f, 500.0f, 0.405f, 4e-6f, -25.0f, 25.0f, 1.0f, 10.0f, -255.0f, 255.0f);
  SVHCurrentSettings cur_set_distal_joint(
    -300.0f, 300.0f, 0.405f, 4e-6f, -25.0f, 25.0f, 1.0f, 10.0f, -255.0f, 255.0f);
  SVHCurrentSettings cur_set_proximal_joint(
    -350.0f, 350.0f, 0.405f, 4e-6f, -25.0f, 25.0f, 1.0f, 10.0f, -255.0f, 255.0f);
  SVHCurrentSettings cur_set_outer_joint(
    -300.0f, 300.0f, 0.405f, 4e-6f, -10.0f, 10.0f, 1.0f, 25.0f, -255.0f, 255.0f);
  SVHCurrentSettings cur_set_finger_spread(
    -500.0f, 500.0f, 0.405f, 4e-6f, -4.0f, 4.0f, 0.7f, 60.0f, -255.0f, 255.0f);


  current_settings[SVH_THUMB_FLEXION] = m_current_settings_given[SVH_THUMB_FLEXION]
                                          ? m_current_settings[SVH_THUMB_FLEXION]
                                          : cur_set_thumb; // thumb flexion
  current_settings[SVH_THUMB_OPPOSITION] = m_current_settings_given[SVH_THUMB_OPPOSITION]
                                             ? m_current_settings[SVH_THUMB_OPPOSITION]
                                             : cur_set_thumb_opposition; // thumb opposition
  current_settings[SVH_INDEX_FINGER_DISTAL] = m_current_settings_given[SVH_INDEX_FINGER_DISTAL]
                                                ? m_current_settings[SVH_INDEX_FINGER_DISTAL]
                                                : cur_set_distal_joint; // index finger distal joint
  current_settings[SVH_INDEX_FINGER_PROXIMAL] =
    m_current_settings_given[SVH_INDEX_FINGER_PROXIMAL]
      ? m_current_settings[SVH_INDEX_FINGER_PROXIMAL]
      : cur_set_proximal_joint; // index finger proximal joint
  current_settings[SVH_MIDDLE_FINGER_DISTAL] =
    m_current_settings_given[SVH_MIDDLE_FINGER_DISTAL]
      ? m_current_settings[SVH_MIDDLE_FINGER_DISTAL]
      : cur_set_distal_joint; // middle finger distal joint
  current_settings[SVH_MIDDLE_FINGER_PROXIMAL] =
    m_current_settings_given[SVH_MIDDLE_FINGER_PROXIMAL]
      ? m_current_settings[SVH_MIDDLE_FINGER_PROXIMAL]
      : cur_set_proximal_joint; // middle finger proximal joint
  current_settings[SVH_RING_FINGER] = m_current_settings_given[SVH_RING_FINGER]
                                        ? m_current_settings[SVH_RING_FINGER]
                                        : cur_set_outer_joint; // ring finger
  current_settings[SVH_PINKY] = m_current_settings_given[SVH_PINKY] ? m_current_settings[SVH_PINKY]
                                                                    : cur_set_outer_joint; // pinky
  current_settings[SVH_FINGER_SPREAD] = m_current_settings_given[SVH_FINGER_SPREAD]
                                          ? m_current_settings[SVH_FINGER_SPREAD]
                                          : cur_set_finger_spread; // finger spread

  return current_settings;
}

//!
//! \brief returns parameters for position settings either the default ones or parameters that have
//! been set from outside
//!
std::vector<SVHPositionSettings> SVHFingerManager::getDefaultPositionSettings(const bool& reset)
{
  std::vector<SVHPositionSettings> position_settings(SVH_DIMENSION);

  // Original conservative settings
  //  SVHPositionSettings pos_set_thumb = {-1.0e6f, 1.0e6f,  3.4e3f, 1.00f, 1e-3f, -500.0f, 500.0f,
  //  0.5f, 0.05f, 0.0f}; SVHPositionSettings pos_set_finger = {-1.0e6f, 1.0e6f,  8.5e3f, 1.00f,
  //  1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f}; SVHPositionSettings pos_set_spread =
  //  {-1.0e6f, 1.0e6f, 17.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};

  // All Fingers with a speed that will close the complete range of the finger in 1 Seconds (except
  // the thumb that will take 4)
  SVHPositionSettings pos_set_thumb_flexion(
    -1.0e6f, 1.0e6f, 65.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.0f, 400.0f);
  SVHPositionSettings pos_set_thumb_opposition(
    -1.0e6f, 1.0e6f, 50.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.1f, 100.0f);
  SVHPositionSettings pos_set_finger_index_distal(
    -1.0e6f, 1.0e6f, 45.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.0f, 40.0f);
  SVHPositionSettings pos_set_finger_index_proximal(
    -1.0e6f, 1.0e6f, 40.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.8f, 0.0f, 1000.0f);
  SVHPositionSettings pos_set_finger_middle_distal(
    -1.0e6f, 1.0e6f, 45.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.0f, 10.0f);
  SVHPositionSettings pos_set_finger_middle_proximal(
    -1.0e6f, 1.0e6f, 40.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.8f, 0.0f, 1000.0f);
  SVHPositionSettings pos_set_finger_ring(
    -1.0e6f, 1.0e6f, 45.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.0f, 100.0f);
  SVHPositionSettings pos_set_finger_pinky(
    -1.0e6f, 1.0e6f, 45.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.0f, 100.0f);
  SVHPositionSettings pos_set_spread(
    -1.0e6f, 1.0e6f, 25.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.0f, 100.0f);

  // OLD (from diagnostics) - All Fingers with a speed that will close the complete range of the
  // finger in 1 Seconds    (except the thumb that wikll take 4)
  //  SVHPositionSettings pos_set_thumb_flexion            (-1.0e6f, 1.0e6f,  65.0e3f, 1.00f, 1e-3f,
  //  -500.0f, 500.0f, 0.5f, 0.05f, 0.0f); SVHPositionSettings pos_set_thumb_opposition
  //  (-1.0e6f, 1.0e6f,  50.0e3f, 1.00f, 1e-3f, -4000.0f, 4000.0f, 0.05f, 0.1f, 0.0f);
  //  SVHPositionSettings pos_set_finger_index_distal      (-1.0e6f, 1.0e6f,  45.0e3f, 1.00f, 1e-3f,
  //  -500.0f, 500.0f, 0.5f, 0.05f, 0.0f); SVHPositionSettings pos_set_finger_index_proximal
  //  (-1.0e6f, 1.0e6f,  40.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.3f, 0.05f, 0.0f);
  //  SVHPositionSettings pos_set_finger_middle_distal     (-1.0e6f, 1.0e6f,  45.0e3f, 1.00f, 1e-3f,
  //  -500.0f, 500.0f, 0.5f, 0.05f, 0.0f); SVHPositisonSettings pos_set_finger_middle_proximal
  //  (-1.0e6f, 1.0e6f,  40.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.3f, 0.05f, 0.0f);
  //  SVHPositionSettings pos_set_finger_ring              (-1.0e6f, 1.0e6f,  45.0e3f, 1.00f, 1e-3f,
  //  -500.0f, 500.0f, 0.5f, 0.05f, 0.0f); SVHPositionSettings pos_set_finger_pinky
  //  (-1.0e6f, 1.0e6f,  45.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f);
  //  SVHPositionSettings pos_set_spread                   (-1.0e6f, 1.0e6f,  25.0e3f, 1.00f, 1e-3f,
  //  -500.0f, 500.0f, 0.5f, 0.05f, 0.0f);


  // Return either the default values or the ones given from outside
  position_settings[SVH_THUMB_FLEXION] = m_position_settings_given[SVH_THUMB_FLEXION]
                                           ? m_position_settings[SVH_THUMB_FLEXION]
                                           : pos_set_thumb_flexion; // thumb flexion
  position_settings[SVH_THUMB_OPPOSITION] = m_position_settings_given[SVH_THUMB_OPPOSITION]
                                              ? m_position_settings[SVH_THUMB_OPPOSITION]
                                              : pos_set_thumb_opposition; // thumb opposition
  position_settings[SVH_INDEX_FINGER_DISTAL] =
    m_position_settings_given[SVH_INDEX_FINGER_DISTAL]
      ? m_position_settings[SVH_INDEX_FINGER_DISTAL]
      : pos_set_finger_index_distal; // index finger distal joint
  position_settings[SVH_INDEX_FINGER_PROXIMAL] =
    m_position_settings_given[SVH_INDEX_FINGER_PROXIMAL]
      ? m_position_settings[SVH_INDEX_FINGER_PROXIMAL]
      : pos_set_finger_index_proximal; // index finger proximal joint
  position_settings[SVH_MIDDLE_FINGER_DISTAL] =
    m_position_settings_given[SVH_MIDDLE_FINGER_DISTAL]
      ? m_position_settings[SVH_MIDDLE_FINGER_DISTAL]
      : pos_set_finger_middle_distal; // middle finger distal joint
  position_settings[SVH_MIDDLE_FINGER_PROXIMAL] =
    m_position_settings_given[SVH_MIDDLE_FINGER_PROXIMAL]
      ? m_position_settings[SVH_MIDDLE_FINGER_PROXIMAL]
      : pos_set_finger_middle_proximal; // middle finger proximal joint
  position_settings[SVH_RING_FINGER] = m_position_settings_given[SVH_RING_FINGER]
                                         ? m_position_settings[SVH_RING_FINGER]
                                         : pos_set_finger_ring; // ring finger
  position_settings[SVH_PINKY] = m_position_settings_given[SVH_PINKY]
                                   ? m_position_settings[SVH_PINKY]
                                   : pos_set_finger_pinky; // pinky
  position_settings[SVH_FINGER_SPREAD] = m_position_settings_given[SVH_FINGER_SPREAD]
                                           ? m_position_settings[SVH_FINGER_SPREAD]
                                           : pos_set_spread; // finger spread

  // Modify the reset speed in case these position settings are meant to be used during the reset
  if (reset)
  {
    for (size_t i = 0; i < SVH_DIMENSION; ++i)
    {
      position_settings[i].dwmx = position_settings[i].dwmx * m_reset_speed_factor;
    }
  }


  return position_settings;
}

void driver_svh::SVHFingerManager::setResetSpeed(const float& speed)
{
  if ((speed >= 0.0) && (speed <= 1.0))
  {
    m_reset_speed_factor = speed;
  }
  else
  {
    SVH_LOG_ERROR_STREAM(
      "SVHFingerManager",
      "The reset speed value given: "
        << speed << " is not valid. Please provide a value between 0.0 and 1.0, default is 0.2");
  }
}

// Converts joint positions of a specific channel from RAD to ticks
int32_t SVHFingerManager::convertRad2Ticks(const SVHChannel& channel, const double& position)
{
  int32_t target_position = static_cast<int32_t>(position / m_ticks2rad[channel]);

  if (m_home_settings[channel].direction > 0)
  {
    target_position += m_position_max[channel];
  }
  else
  {
    target_position += m_position_min[channel];
  }

  return target_position;
}

// Converts Joint ticks of a specific channel back to RAD removing its offset in the process
double SVHFingerManager::convertTicks2Rad(const SVHChannel& channel, const int32_t& ticks)
{
  int32_t cleared_position_ticks;

  if (m_home_settings[channel].direction > 0)
  {
    cleared_position_ticks = ticks - m_position_max[channel];
  }
  else
  {
    cleared_position_ticks = ticks - m_position_min[channel];
  }

  return static_cast<double>(cleared_position_ticks * m_ticks2rad[channel]);
}

// Converts joint efforts of a specific channel from force [N] to current [mA]
uint16_t SVHFingerManager::convertNtomA(const SVHChannel& channel, const double& effort)
{
  uint16_t current;
  if (SVHController::CHANNEL_EFFORT_CONSTANTS[channel][0] != 0)
  {
    // y = a*x + b -->  x = (y-b) / a
    // y = effort and x = current
    current = static_cast<uint16_t>(
      std::round((effort - SVHController::CHANNEL_EFFORT_CONSTANTS[channel][1]) /
                 SVHController::CHANNEL_EFFORT_CONSTANTS[channel][0]));
  }
  else
  {
    current =
      static_cast<uint16_t>(m_max_current_percentage * m_diagnostic_current_maximum[channel]);
  }

  return current;
}

// Converts joint effort of a specific channel from current [mA] to force [N]
double SVHFingerManager::convertmAtoN(const SVHChannel& channel, const int16_t& current)
{
  float effort;
  // y = a*x + b
  // y = effort and x = current
  effort = SVHController::CHANNEL_EFFORT_CONSTANTS[channel][0] * std::abs(current) +
           SVHController::CHANNEL_EFFORT_CONSTANTS[channel][1];

  return effort;
}

// Check bounds of target positions
bool SVHFingerManager::isInsideBounds(const SVHChannel& channel, const int32_t& target_position)
{
  // Switched off channels will always be reported as inside bounds
  if (m_is_switched_off[channel] || ((target_position >= m_position_min[channel]) &&
                                     (target_position <= m_position_max[channel])))
  {
    return true;
  }
  else
  {
    SVH_LOG_WARN_STREAM("SVHFingerManager",
                        "Channel" << channel << " : "
                                  << SVHController::m_channel_description[channel]
                                  << " Target: " << target_position << "("
                                  << convertTicks2Rad(channel, target_position) << "rad)"
                                  << " is out of bounds! [" << m_position_min[channel] << "/"
                                  << m_position_max[channel] << "]");
    return false;
  }
}

void SVHFingerManager::requestControllerState()
{
  m_controller->requestControllerState();
}

void SVHFingerManager::setResetTimeout(const int& reset_timeout)
{
  m_reset_timeout =
    (reset_timeout > 0) ? std::chrono::seconds(reset_timeout) : std::chrono::seconds(0);
}

bool SVHFingerManager::setMaxForce(float max_force)
{
  if (max_force > 0 && max_force <= 1)
  {
    m_max_current_percentage = max_force;
    return true;
  }
  else
  {
    SVH_LOG_WARN_STREAM(
      "SVHFingerManager",
      "Maximal Force / current should be in the range of [0,1], was set to: " << max_force);
    return false;
  }
}

float SVHFingerManager::setForceLimit(const SVHChannel& channel, float force_limit)
{
  uint16_t current;
  current = convertNtomA(channel, force_limit);

  SVHCurrentSettings current_settings;

  current_settings = m_current_settings[channel];

  current_settings.wmx = current;
  current_settings.wmn = -static_cast<float>(current);

  if (setCurrentSettings(channel, current_settings))
  {
    return force_limit;
  }
  else
  {
    return 0.0;
  }
}


SVHFirmwareInfo SVHFingerManager::getFirmwareInfo(const std::string& dev_name,
                                                  const unsigned int& retry_count)
{
  // If firmware was read out befor do not ask for new firmware
  if (m_firmware_info.version_major == 0 && m_firmware_info.version_major == 0)
  {
    bool was_connected = true;

    if (!m_connected)
    {
      was_connected = false;
      if (!m_controller->connect(dev_name))
      {
        SVH_LOG_ERROR_STREAM("SVHFingerManager", "Connection FAILED! Device could NOT be opened");
        m_firmware_info.version_major = 0;
        m_firmware_info.version_minor = 0;
        return m_firmware_info;
      }
    }

    // As the firmware info takes longer we need to disable the polling during the request of the
    // firmware information
    m_poll_feedback = false;
    if (m_feedback_thread.joinable())
    {
      m_feedback_thread.join();
    }

    unsigned int num_retries = retry_count;
    do
    {
      // Tell the hardware to get the newest firmware information
      m_controller->requestFirmwareInfo();
      // Just wait a tiny amount
      std::this_thread::sleep_for(std::chrono::microseconds(100000));
      // Get the Version number if received yet, else 0.0
      m_firmware_info = m_controller->getFirmwareInfo();
      --num_retries;

      if (m_firmware_info.version_major == 0 && m_firmware_info.version_major == 0)
      {
        SVH_LOG_ERROR_STREAM("SVHFingerManager",
                             "Getting Firmware Version failed,.Retrying, count: " << num_retries);
      }
    } while (num_retries > 0 && m_firmware_info.version_major == 0 &&
             m_firmware_info.version_major == 0);

    // Start the feedback process aggain
    m_poll_feedback   = true;
    m_feedback_thread = std::thread(&SVHFingerManager::pollFeedback, this);

    if (!was_connected)
    {
      m_controller->disconnect();
    }
  }

  // Note that the Firmware will also be printed to the console by the controller. So in case you
  // just want to know it no further action is required
  return m_firmware_info;
}

void SVHFingerManager::pollFeedback()
{
  while (m_poll_feedback)
  {
    if (isConnected())
    {
      requestControllerFeedback(SVH_ALL);
    }
    else
    {
      SVH_LOG_WARN_STREAM("SVHFeedbackPollingThread", "SCHUNK five finger hand is not connected!");
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

} // namespace driver_svh
