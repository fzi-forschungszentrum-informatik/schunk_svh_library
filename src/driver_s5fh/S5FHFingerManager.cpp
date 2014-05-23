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
#include <driver_s5fh/Logging.h>
#include <driver_s5fh/S5FHFingerManager.h>

#include <icl_core/TimeStamp.h>

namespace driver_s5fh {

S5FHFingerManager::S5FHFingerManager(const bool &autostart,const std::string &dev_name) :
  m_controller(new S5FHController()),
  m_feedback_thread(NULL),
  m_connected(false),
  m_homing_timeout(10),
  m_home_settings(0),
  m_ticks2rad(0),
  m_position_min(std::vector<int32_t>(eS5FH_DIMENSION, 0)),
  m_position_max(std::vector<int32_t>(eS5FH_DIMENSION, 0)),
  m_position_home(std::vector<int32_t>(eS5FH_DIMENSION, 0)),
  m_is_homed(std::vector<bool>(eS5FH_DIMENSION, false)),
  m_movement_state(eST_DEACTIVATED)
{
  // load home position default parameters
  setHomePositionDefaultParameters();

  // set default reset order of all channels
  m_reset_order.resize(eS5FH_DIMENSION);
  m_reset_order[0] = eS5FH_INDEX_FINGER_PROXIMAL;
  m_reset_order[1] = eS5FH_MIDDLE_FINGER_PROXIMAL;
  m_reset_order[2] = eS5FH_THUMB_OPPOSITION;
  m_reset_order[3] = eS5FH_THUMB_FLEXION;
  m_reset_order[4] = eS5FH_FINGER_SPREAD;
  m_reset_order[5] = eS5FH_MIDDLE_FINGER_DISTAL;
  m_reset_order[6] = eS5FH_INDEX_FINGER_DISTAL;
  m_reset_order[7] = eS5FH_RING_FINGER;
  m_reset_order[8] = eS5FH_PINKY;

  // Order is determined by the channel enum
  m_reset_current_factor.resize(eS5FH_DIMENSION);
  m_reset_current_factor[eS5FH_THUMB_FLEXION]=          0.75;
  m_reset_current_factor[eS5FH_THUMB_OPPOSITION]=       0.75;
  m_reset_current_factor[eS5FH_INDEX_FINGER_DISTAL]=    0.75;
  m_reset_current_factor[eS5FH_INDEX_FINGER_PROXIMAL]=  0.75;
  m_reset_current_factor[eS5FH_MIDDLE_FINGER_DISTAL]=   0.75;
  m_reset_current_factor[eS5FH_MIDDLE_FINGER_PROXIMAL]= 0.75;
  m_reset_current_factor[eS5FH_RING_FINGER]=            0.75;
  m_reset_current_factor[eS5FH_PINKY]=                  0.75;
  m_reset_current_factor[eS5FH_FINGER_SPREAD]=          0.5;  // needs a lower current threshold to properly reset


  //#ifdef _IC_BUILDER_ICL_COMM_WEBSOCKET_
    try
    {
      // TODO: Make the socket adress non const :)
     m_ws_broadcaster = boost::shared_ptr<icl_comm::websocket::WsBroadcaster>(new icl_comm::websocket::WsBroadcaster(icl_comm::websocket::WsBroadcaster::eRT_S5FH,"/tmp/ws_broadcaster"));
     m_ws_broadcaster->robot->setInputToRadFactor(1);
    }
    catch (icl_comm::websocket::SocketException e)
    {
      std::cout << e.what() << std::endl;
    }
  //#endif // _IC_BUILDER_ICL_COMM_WEBSOCKET_


  // Try First Connect and Reset of all Fingers if autostart is enabled
  if (autostart && connect(dev_name))
  {
    resetChannel(eS5FH_ALL);
    LOGGING_INFO_C(DriverS5FH, S5FHFingerManager, "Driver Autostart succesfull! Input can now be sent. Have a safe and productive day" << endl);
  }

}

S5FHFingerManager::~S5FHFingerManager()
{
  disconnect();

  if (m_controller != NULL)
  {
    delete m_controller;
    m_controller = NULL;
  }
}

bool S5FHFingerManager::connect(const std::string &dev_name)
{
  if (m_connected)
  {
    disconnect();
  }

  if (m_controller != NULL)
  {
    if (m_controller->connect(dev_name))
    {
      // initialize feedback polling thread
      m_feedback_thread = new S5FHFeedbackPollingThread(icl_core::TimeSpan::createFromMSec(100), this);

      // load default position settings before the fingers are resetted
      std::vector<S5FHPositionSettings> default_position_settings
          = getPositionSettingsDefaultResetParameters();

      // load default current settings
      std::vector<S5FHCurrentSettings> default_current_settings
          = getCurrentSettingsDefaultParameters();

      m_controller->disableChannel(eS5FH_ALL);

      // initialize all channels
      for (size_t i = 0; i < eS5FH_DIMENSION; ++i)
      {
        // request controller feedback
        m_controller->requestControllerFeedback(static_cast<S5FHCHANNEL>(i));

        // set position settings
        m_controller->setPositionSettings(static_cast<S5FHCHANNEL>(i), default_position_settings[i]);

        // set current settings
        m_controller->setCurrentSettings(static_cast<S5FHCHANNEL>(i), default_current_settings[i]);
      }

      // check for correct response from hardware controller
      icl_core::TimeStamp start_time = icl_core::TimeStamp::now();
      bool timeout = false;
      while (!timeout && !m_connected)
      {
        unsigned int send_count = m_controller->getSentPackageCount();
        unsigned int received_count = m_controller->getReceivedPackageCount();
        if (send_count == received_count)
        {
          m_connected = true;
          LOGGING_INFO_C(DriverS5FH, S5FHFingerManager, "Successfully established connection to SCHUNK five finger hand." << endl
                          << "Send packages = " << send_count << ", received packages = " << received_count << endl);

        }
        LOGGING_DEBUG_C(DriverS5FH, S5FHFingerManager, "Try to connect to SCHUNK five finger hand: Send packages = " << send_count << ", received packages = " << received_count << endl);

        // check for timeout
        if ((icl_core::TimeStamp::now() - start_time).tsSec() > 5.0)
        {
          timeout = true;
          LOGGING_ERROR_C(DriverS5FH, S5FHFingerManager, "Connection timeout! Could not connect to SCHUNK five finger hand." << endl
                          << "Send packages = " << send_count << ", received packages = " << received_count << endl);
        }

        icl_core::os::usleep(50000);
      }

      if (m_connected)
      {
        // start feedback polling thread
        if (m_feedback_thread != NULL)
        {
          m_feedback_thread->start();
        }
      }
    }
  }

  return m_connected;
}

void S5FHFingerManager::disconnect()
{
  m_connected = false;

  if (m_feedback_thread != NULL)
  {
    // wait until thread has stopped
    m_feedback_thread->stop();
    m_feedback_thread->join();

    delete m_feedback_thread;
    m_feedback_thread = NULL;
  }

  if (m_controller != NULL)
  {
    m_controller->disconnect();
  }
}

//! reset function for a single finger
bool S5FHFingerManager::resetChannel(const S5FHCHANNEL &channel)
{
  if (m_connected)
  {
    // reset all channels
    if (channel == eS5FH_ALL)
    {
      setMovementState(eST_RESETTING);

      bool reset_all_success = true;
      for (size_t i = 0; i < eS5FH_DIMENSION; ++i)
      {
        // try three times to reset each finger
        size_t max_reset_counter = 3;
        bool reset_success = false;
        while (!reset_success && max_reset_counter > 0)
        {
          S5FHCHANNEL channel = static_cast<S5FHCHANNEL>(m_reset_order[i]);
          reset_success = resetChannel(channel);
          max_reset_counter--;
        }

        LOGGING_INFO_C(DriverS5FH, resetChannel, "Channel " << m_reset_order[i] << " reset success = " << reset_success << endl);

        // set all reset flag
        reset_all_success = reset_all_success && reset_success;
      }
      return reset_all_success;
    }
    else if (channel > eS5FH_ALL && eS5FH_ALL < eS5FH_DIMENSION)
    {
      LOGGING_DEBUG_C(DriverS5FH, S5FHFingerManager, "Start homing channel " << channel << endl);

      LOGGING_DEBUG_C(DriverS5FH, S5FHFingerManager, "Setting reset position values for controller of channel " << channel << endl);
      m_controller->setPositionSettings(channel, getPositionSettingsDefaultResetParameters()[channel]);

      // reset homed flag
      m_is_homed[channel] = false;

      // read default home settings for channel
      HomeSettings home = m_home_settings[channel];

      S5FHPositionSettings pos_set;
      S5FHCurrentSettings cur_set;
      m_controller->getPositionSettings(channel, pos_set);
      m_controller->getCurrentSettings(channel, cur_set);

      // find home position
      m_controller->disableChannel(eS5FH_ALL);
      int32_t position = 0;

      if (home.direction > 0)
      {
        position = static_cast<int32_t>(pos_set.wmx);
      }
      else
      {
        position = static_cast<int32_t>(pos_set.wmn);
      }

      LOGGING_INFO_C(DriverS5FH, S5FHFingerManager, "Driving channel " << channel << " to hardstop. Detection thresholds: Current MIN: "<< m_reset_current_factor[channel] * cur_set.wmn << "mA MAX: "<< m_reset_current_factor[channel] * cur_set.wmx <<"mA" << endl);

      m_controller->setControllerTarget(channel, position);
      m_controller->enableChannel(channel);

      S5FHControllerFeedback control_feedback_previous;
      S5FHControllerFeedback control_feedback;

      // initialize timeout
      icl_core::TimeStamp start_time = icl_core::TimeStamp::now();

      for (size_t hit_count = 0; hit_count < 10; )
      {
        m_controller->setControllerTarget(channel, position);
        //m_controller->requestControllerFeedback(channel);
        m_controller->getControllerFeedback(channel, control_feedback);

        if ((m_reset_current_factor[channel] * cur_set.wmn >= control_feedback.current) || (control_feedback.current >= m_reset_current_factor[channel] * cur_set.wmx))
        {
          hit_count++;
        }
        else if (hit_count > 0)
        {
          hit_count--;
        }

        // check for time out: Abort, if position does not change after homing timeout.
        if ((icl_core::TimeStamp::now() - start_time).tsSec() > m_homing_timeout)
        {
          m_controller->disableChannel(eS5FH_ALL);
          LOGGING_ERROR_C(DriverS5FH, S5FHFingerManager, "Timeout: Aborted finding home position for channel " << channel << endl);
          return false;
        }

        // reset time of position changes
        if (control_feedback.position != control_feedback_previous.position)
        {
          start_time = icl_core::TimeStamp::now();
        }

        // save previous control feedback
        control_feedback_previous = control_feedback;
      }

      LOGGING_DEBUG_C(DriverS5FH, S5FHFingerManager, "Hit counter of " << channel << " reached." << endl);

      m_controller->disableChannel(eS5FH_ALL);

      // set reference values
      m_position_min[channel] = static_cast<int32_t>(control_feedback.position + home.minimumOffset);
      m_position_max[channel] = static_cast<int32_t>(control_feedback.position + home.maximumOffset);
      m_position_home[channel] = static_cast<int32_t>(control_feedback.position + home.idlePosition);
      LOGGING_DEBUG_C(DriverS5FH, S5FHFingerManager, "Channel " << channel << " min pos = " << m_position_min[channel]
                      << " max pos = " << m_position_max[channel] << " home pos = " << m_position_home[channel] << endl);

      position = static_cast<int32_t>(control_feedback.position + home.idlePosition);

      // go to idle position
      m_controller->enableChannel(channel);
      while (true)
      {
        m_controller->setControllerTarget(channel, position);
        //m_controller->requestControllerFeedback(channel);
        m_controller->getControllerFeedback(channel, control_feedback);

        if (abs(position - control_feedback.position) < 1000)
        {
          break;
        }
      }
      m_controller->disableChannel(eS5FH_ALL);

      LOGGING_DEBUG_C(DriverS5FH, S5FHFingerManager, "Restoring default position values for controller of channel " << channel << endl);
      m_controller->setPositionSettings(channel, getPositionSettingsDefaultParameters()[channel]);


      m_is_homed[channel] = true;
      //#ifdef _IC_BUILDER_ICL_COMM_WEBSOCKET_
      if (m_ws_broadcaster)
      {
        m_ws_broadcaster->robot->setJointHomed(true,channel);
        m_ws_broadcaster->sendState();
      }
      //#endif // _IC_BUILDER_ICL_COMM_WEBSOCKET_

      LOGGING_DEBUG_C(DriverS5FH, S5FHFingerManager, "End homing of channel " << channel << endl);

      return true;
    }
    else
    {
      LOGGING_ERROR_C(DriverS5FH, S5FHFingerManager, "Channel " << channel << " is out side of bounds!" << endl);
      return false;
    }
  }
  else
  {
    LOGGING_ERROR_C(DriverS5FH, S5FHFingerManager, "Could not reset channel " << channel << ": No connection to SCHUNK five finger hand!" << endl);
    return false;
  }
}

//! enables controller of channel
bool S5FHFingerManager::enableChannel(const S5FHCHANNEL &channel)
{
  if (isConnected() && isHomed(channel))
  {
    if (channel == eS5FH_ALL)
    {
      for (size_t i = 0; i < eS5FH_DIMENSION; ++i)
      {
        // Just for safety, enable chanels in the same order as we have resetted them (otherwise developers might geht confused)
        S5FHCHANNEL real_channel = static_cast<S5FHCHANNEL>(m_reset_order[i]);
        m_controller->enableChannel(real_channel);
      }
    }
    else if (channel > eS5FH_ALL && eS5FH_ALL < eS5FH_DIMENSION)
    {
      m_controller->enableChannel(channel);
      //#ifdef _IC_BUILDER_ICL_COMM_WEBSOCKET_
      if (m_ws_broadcaster)
      {
        m_ws_broadcaster->robot->setJointEnabled(true,channel);
        m_ws_broadcaster->sendState();
      }
      //#endif // _IC_BUILDER_ICL_COMM_WEBSOCKET_

      setMovementState(eST_PARTIALLY_ENABLED);
      if (isEnabled(eS5FH_ALL))
      {
        setMovementState(eST_ENABLED);
      }

    }
    return true;
  }
  return false;
}

//! disables controller of channel
void S5FHFingerManager::disableChannel(const S5FHCHANNEL &channel)
{
  if (channel == eS5FH_ALL)
  {
    for (size_t i = 0; i < eS5FH_DIMENSION; ++i)
    {
      m_controller->disableChannel(static_cast<S5FHCHANNEL>(i));
    }
  }
  else
  {
    m_controller->disableChannel(channel);
    //#ifdef _IC_BUILDER_ICL_COMM_WEBSOCKET_
    if (m_ws_broadcaster)
    {
      m_ws_broadcaster->robot->setJointEnabled(false,channel);
      m_ws_broadcaster->sendState();
    }
    //#endif // _IC_BUILDER_ICL_COMM_WEBSOCKET_
    setMovementState(eST_PARTIALLY_ENABLED);

    bool all_disabled = true;
    for (size_t i = 0; i < eS5FH_DIMENSION; ++i)
    {
      all_disabled = all_disabled && !isEnabled(static_cast<S5FHCHANNEL>(i));
    }
    if (all_disabled)
    {
      setMovementState(eST_DEACTIVATED);
    }

  }
}

bool S5FHFingerManager::requestControllerFeedbackAllChannels()
{
  if (isConnected())
  {
    m_controller->requestControllerFeedbackAllChannels();
    return true;
  }

  return false;
}

bool S5FHFingerManager::requestControllerFeedback(const S5FHCHANNEL &channel)
{
  if (isConnected() && isHomed(channel) && isEnabled(channel))
  {
    m_controller->requestControllerFeedback(channel);
    return true;
  }

  LOGGING_WARNING_C(DriverS5FH, S5FHFingerManager, "Channel " << channel << " is not homed or is not enabled!" << endl);
  return false;
}

//! returns actual position value for given channel
bool S5FHFingerManager::getPosition(const S5FHCHANNEL &channel, double &position)
{
  S5FHControllerFeedback controller_feedback;
  if ((channel >=0 && channel < eS5FH_DIMENSION) && isHomed(channel) && m_controller->getControllerFeedback(channel, controller_feedback))
  {
    int32_t cleared_position_ticks = controller_feedback.position;

    if (m_home_settings[channel].direction > 0)
    {
      cleared_position_ticks -= m_position_max[channel];
    }
    else
    {
      cleared_position_ticks -= m_position_min[channel];
    }

    position = static_cast<double>(cleared_position_ticks * m_ticks2rad[channel]);

    // Dirty Hack: If controller drives to a negative position, we cannot get out because inputs smaller than 0 will be ignored
    if (position < 0)
    {
      position = 0.0;
    }

    LOGGING_DEBUG_C(DriverS5FH, S5FHFingerManager, "Channel " << channel << ": position_ticks = " << controller_feedback.position
                    << " | cleared_position_ticks = " << cleared_position_ticks << " | position rad = " << position << endl);
    return true;
  }
  else
  {
    //LOGGING_WARNING_C(DriverS5FH, S5FHFingerManager, "Could not get postion for channel " << channel << endl);
    return false;
  }
}



//#ifdef _IC_BUILDER_ICL_COMM_WEBSOCKET_
void S5FHFingerManager::updateWebSocket()
{
  if (m_ws_broadcaster)
  {
    double position;
    //double current // will be implemented later in the WS Broadcaster
    for (size_t i = 0; i < eS5FH_DIMENSION; ++i)
    {
      // NOTE: Although the call to getPosition and current cann fail fue to multiple reason, the only one we would encounter with these calls is a
      // non-homed finger. So it is quite safe to assume that the finger is NOT homed if these calls fail and we can safe multiple acces to the homed variable

      if (getPosition(static_cast<S5FHCHANNEL>(i),position)) // && (getCurrent(i,current))
      {
          m_ws_broadcaster->robot->setJointPosition(position,i);
        //m_ws_broadcaster>robot>setJpintCurrent(current,i);
      }
      else
      {
        m_ws_broadcaster->robot->setJointHomed(false,i);
      }

      m_ws_broadcaster->sendState();
    }
  }
}
//#endif // _IC_BUILDER_ICL_COMM_WEBSOCKET_





//! returns actual current value for given channel
bool S5FHFingerManager::getCurrent(const S5FHCHANNEL &channel, double &current)
{
  S5FHControllerFeedback controller_feedback;
  if (isHomed(channel) && m_controller->getControllerFeedback(channel, controller_feedback))
  {
    current = controller_feedback.current;
    return true;
  }
  else
  {
    LOGGING_WARNING_C(DriverS5FH, S5FHFingerManager, "Could not get current for channel " << channel << endl);
    return false;
  }
}

bool S5FHFingerManager::getCurrentControllerParams(const S5FHCHANNEL &channel, S5FHCurrentSettings &current_settings)
{
  return m_controller->getCurrentSettings(channel, current_settings);
}

bool S5FHFingerManager::getPositionControllerParams(const S5FHCHANNEL &channel, S5FHPositionSettings &position_settings)
{
  return m_controller->getPositionSettings(channel, position_settings);
}

//! set all target positions at once
bool S5FHFingerManager::setAllTargetPositions(const std::vector<double>& positions)
{
  if (isConnected())
  {
    // check size of position vector
    if (positions.size() == eS5FH_DIMENSION)
    {
      // create target positions vector
      std::vector<int32_t> target_positions(eS5FH_DIMENSION, 0);

      bool reject_command = false;
      for (size_t i = 0; i < eS5FH_DIMENSION; ++i)
      {
        S5FHCHANNEL channel = static_cast<S5FHCHANNEL>(i);

        // enable all homed and disabled channels
        if (isHomed(channel) && !isEnabled(channel))
        {
          enableChannel(channel);
        }

        // convert all channels to ticks
        target_positions[channel] = convertRad2Ticks(channel, positions[channel]);

        // check for out of bounds
        if (!isInsideBounds(channel, target_positions[channel]))
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
        LOGGING_WARNING_C(DriverS5FH, S5FHFingerManager, "Could not set target position vector: At least one channel is out of bounds!" << endl);
        return false;
      }

    }
    else
    {
      LOGGING_WARNING_C(DriverS5FH, S5FHFingerManager, "Size of target position vector wrong: size = " << positions.size() << " expected size = " << (int)eS5FH_DIMENSION << endl);
      return false;
    }
  }
  else
  {
    LOGGING_ERROR_C(DriverS5FH, S5FHFingerManager, "Could not set target position vector: No connection to SCHUNK five finger hand!" << endl);
    return false;
  }
}

//! set target position of a single channel
bool S5FHFingerManager::setTargetPosition(const S5FHCHANNEL &channel, double position, double current)
{
  if (isConnected())
  {
    if (isHomed(channel))
    {
      int32_t target_position = convertRad2Ticks(channel, position);

      LOGGING_DEBUG_C(DriverS5FH, setTargetPosition, "Target position for channel " << channel << " = " << target_position << endl);

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
        LOGGING_ERROR_C(DriverS5FH, S5FHFingerManager, "Target position for channel " << channel << " out of bounds!" << endl);
        return false;
      }
    }
    else
    {
      LOGGING_ERROR_C(DriverS5FH, S5FHFingerManager, "Could not set target position for channel " << channel << ": Reset first!" << endl);
      return false;
    }
  }
  else
  {
    LOGGING_ERROR_C(DriverS5FH, S5FHFingerManager, "Could not set target position for channel " << channel << ": No connection to SCHUNK five finger hand!" << endl);
    return false;
  }
}

//! overwrite current parameters
bool S5FHFingerManager::setCurrentControllerParams(const S5FHCHANNEL &channel, const S5FHCurrentSettings &current_settings)
{
  m_controller->setCurrentSettings(channel, current_settings);
  return true;
}

//! overwrite position parameters
bool S5FHFingerManager::setPositionControllerParams(const S5FHCHANNEL &channel, const S5FHPositionSettings &position_settings)
{
  m_controller->setPositionSettings(channel, position_settings);
  return true;
}

//! return enable flag
bool S5FHFingerManager::isEnabled(const S5FHCHANNEL &channel)
{
  if (channel==eS5FH_ALL)
  {
    bool all_enabled = true;
    for (size_t i = 0; i < eS5FH_DIMENSION; ++i)
    {
      all_enabled = all_enabled && m_controller->isEnabled(static_cast<S5FHCHANNEL>(i));
    }

    return all_enabled;
  }
  return m_controller->isEnabled(channel);
}

//! return homed flag
bool S5FHFingerManager::isHomed(const S5FHCHANNEL &channel)
{
  if (channel == eS5FH_ALL)
  {
    bool all_homed = true;
    for (size_t i = 0; i < eS5FH_DIMENSION; ++i)
    {
      all_homed = all_homed && m_is_homed[i];
    }

    return all_homed;
  }
  return m_is_homed[channel];
}

void S5FHFingerManager::setMovementState(const S5FHFingerManager::MovementState &state)
{
  m_movement_state = state;

  //#ifdef _IC_BUILDER_ICL_COMM_WEBSOCKET_
  if (m_ws_broadcaster)
  {
    m_ws_broadcaster->robot->setMovementState(state);
    m_ws_broadcaster->sendState();
  }
  //#endif // _IC_BUILDER_ICL_COMM_WEBSOCKET_


}

//!
//! \brief set default parameters for home position
//!
void S5FHFingerManager::setHomePositionDefaultParameters()
{
  m_home_settings.resize(eS5FH_DIMENSION);
                    // direction, minimum offset, maximum offset, idle position
  HomeSettings home_set_thumb_flexion   = {+1, -175.0e3f,  -5.0e3f, -15.0e3f};  // RE17, thumb flexion
  HomeSettings home_set_thumb_oppsition = {+1, -105.0e3f,  -5.0e3f, -15.0e3f};  // RE17, thumb opposition
  HomeSettings home_set_finger_distal   = {+1,  -47.0e3f,  -2.0e3f,  -8.0e3f};  // RE10, index finger distal joint
  HomeSettings home_set_finger_proximal = {-1,    2.0e3f,  42.0e3f,   8.0e3f};  // RE13, index finger proximal joint   --> Values based on the "limits" described by the hardware table
  //HomeSettings home_set_finger_proximal = {-1,    2.0e3f,  47.0e3f,   8.0e3f}; // Better Looking and more homogeneous maximum ;)
  HomeSettings home_set_ring_finger     = home_set_finger_distal; //{+1,  -47.0e3f,  -2.0e3f,  -8.0e3f};  // RE10, ring finger
  HomeSettings home_set_pinky           = home_set_finger_distal; //{+1,  -47.0e3f,  -2.0e3f,  -8.0e3f};  // RE10, pinky
  HomeSettings home_set_finger_spread   = {+1,  -47.0e3f,  -2.0e3f,  -25.0e3f}; //{+1,  -25.0e3f,  -2.0e3f,  -15.0e3f};  //   // RE13, finger spread

  m_home_settings[0] = home_set_thumb_flexion;    // thumb flexion
  m_home_settings[1] = home_set_thumb_oppsition;  // thumb opposition
  m_home_settings[2] = home_set_finger_distal;    // index finger distal joint
  m_home_settings[3] = home_set_finger_proximal;  // index finger proximal joint
  m_home_settings[4] = home_set_finger_distal;    // middle finger distal joint
  m_home_settings[5] = home_set_finger_proximal;  // middle finger proximal joint
  m_home_settings[6] = home_set_ring_finger;      // ring finger
  m_home_settings[7] = home_set_pinky;            // pinky
  m_home_settings[8] = home_set_finger_spread;    // finger spread

  // calculate factors and offset for ticks to rad conversion
  float range_rad_data[eS5FH_DIMENSION] = { 0.97, 0.99, 1.33, 0.8, 1.33, 0.8, 0.98, 0.98, 0.58 };
  //float range_rad_data[eS5FH_DIMENSION] = { 0.97, 0.99, 1.33, 0.98, 1.33, 0.98, 0.98, 0.98, 0.58 };
  std::vector<float> range_rad(&range_rad_data[0], &range_rad_data[0] + eS5FH_DIMENSION);

  m_ticks2rad.resize(eS5FH_DIMENSION, 0.0);
  for (size_t i = 0; i < eS5FH_DIMENSION; ++i)
  {
    float range_ticks = m_home_settings[i].maximumOffset - m_home_settings[i].minimumOffset;
    m_ticks2rad[i] = range_rad[i] / range_ticks * (-m_home_settings[i].direction);

    // debug
    //std::cout << "Channel " << i << ": ticks2rad factor = " << m_ticks2rad[i] << std::endl;
  }
}

//!
//! \brief returns default parameters for current settings
//!
std::vector<S5FHCurrentSettings> S5FHFingerManager::getCurrentSettingsDefaultParameters()
{
  std::vector<S5FHCurrentSettings> default_current_settings(eS5FH_DIMENSION);
  // curr min, Curr max,ky(error output scaling),dt(time base),imn (integral windup min), imx (integral windup max), kp,ki,umn,umx (output limter)
  //S5FHCurrentSettings cur_set_thumb          = {-350.0f, 350.0f, 0.405f, 4e-6f, -300.0f, 300.0f, 0.850f, 85.0f, -354.0f, 354.0f}; // old: {-191.0f, 191.0f, 0.405f, 4e-6f, -300.0f, 300.0f, 0.850f, 85.0f, -254.0f, 254.0f};
  S5FHCurrentSettings cur_set_thumb          = {-400.0f, 400.0f, 0.405f, 4e-6f, -300.0f, 300.0f, 0.850f, 85.0f, -500.0f, 500.0f};
  S5FHCurrentSettings cur_set_thumb_opposition = {-400.0f, 400.0f, 0.405f, 4e-6f, -400.0f, 400.0f, 0.90f, 85.0f, -800.0f, 800.0f};
  S5FHCurrentSettings cur_set_distal_joint   = {-176.0f, 176.0f, 0.405f, 4e-6f, -300.0f, 300.0f, 0.850f, 85.0f, -254.0f, 254.0f};
  S5FHCurrentSettings cur_set_proximal_joint = {-167.0f, 167.0f, 0.405f, 4e-6f, -300.0f, 300.0f, 0.850f, 85.0f, -254.0f, 254.0f};
  //S5FHCurrentSettings cur_set_finger_spread  = {-200.0f, 200.0f, 0.405f, 4e-6f, -300.0f, 300.0f, 0.850f, 85.0f, -254.0f, 254.0f}; // old: {-167.0f, 167.0f, 0.405f, 4e-6f, -300.0f, 300.0f, 0.850f, 85.0f, -254.0f, 254.0f};
  S5FHCurrentSettings cur_set_finger_spread  = {-200.0f, 200.0f, 0.405f, 4e-6f, -300.0f, 300.0f, 0.850f, 85.0f, -400.0f, 400.0f};

  default_current_settings[0] = cur_set_thumb_opposition;          // thumb flexion
  default_current_settings[1] = cur_set_thumb_opposition;          // thumb opposition
  default_current_settings[2] = cur_set_distal_joint;   // index finger distal joint
  default_current_settings[3] = cur_set_proximal_joint; // index finger proximal joint
  default_current_settings[4] = cur_set_distal_joint;   // middle finger distal joint
  default_current_settings[5] = cur_set_proximal_joint; // middle finger proximal joint
  default_current_settings[6] = cur_set_distal_joint;   // ring finger
  default_current_settings[7] = cur_set_distal_joint;   // pinky
  default_current_settings[8] = cur_set_finger_spread;  // finger spread

  return default_current_settings;
}


//!
//! \brief returns default RESET parameters for position settings
//!
std::vector<S5FHPositionSettings> S5FHFingerManager::getPositionSettingsDefaultResetParameters()
{
  std::vector<S5FHPositionSettings> default_position_settings(eS5FH_DIMENSION);


  //S5FHPositionSettings pos_set_thumb = {-1.0e6f, 1.0e6f,  3.4e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//  S5FHPositionSettings pos_set_finger = {-1.0e6f, 1.0e6f,  8.5e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//  S5FHPositionSettings pos_set_spread = {-1.0e6f, 1.0e6f, 17.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};

  S5FHPositionSettings pos_set_thumb = {-1.0e6f, 1.0e6f,  10.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
  S5FHPositionSettings pos_set_finger = {-1.0e6f, 1.0e6f, 15.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
  S5FHPositionSettings pos_set_spread = {-1.0e6f, 1.0e6f, 17.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};


  default_position_settings[0] = pos_set_thumb;   // thumb flexion
  default_position_settings[1] = pos_set_thumb;   // thumb opposition
  default_position_settings[2] = pos_set_finger;  // index finger distal joint
  default_position_settings[3] = pos_set_finger;  // index finger proximal joint
  default_position_settings[4] = pos_set_finger;  // middle finger distal joint
  default_position_settings[5] = pos_set_finger;  // middle finger proximal joint
  default_position_settings[6] = pos_set_finger;  // ring finger
  default_position_settings[7] = pos_set_finger;  // pinky
  default_position_settings[8] = pos_set_spread;  // finger spread

  return default_position_settings;
}


//!
//! \brief returns default parameters for position settings
//!
std::vector<S5FHPositionSettings> S5FHFingerManager::getPositionSettingsDefaultParameters()
{
  std::vector<S5FHPositionSettings> default_position_settings(eS5FH_DIMENSION);
  // Was this:
//  S5FHPositionSettings pos_set_thumb = {-1.0e6f, 1.0e6f,  3.4e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//  S5FHPositionSettings pos_set_finger = {-1.0e6f, 1.0e6f,  8.5e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//  S5FHPositionSettings pos_set_spread = {-1.0e6f, 1.0e6f, 17.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};

    // All Fingers 0.5rad/sec except the fingers (1.5)
//  S5FHPositionSettings pos_set_thumb_flexion =          {-1.0e6f, 1.0e6f,  26.288e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//  S5FHPositionSettings pos_set_thumb_opposition =       {-1.0e6f, 1.0e6f,  15.151e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//  S5FHPositionSettings pos_set_finger_index_distal =    {-1.0e6f, 1.0e6f,  16.917e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//  S5FHPositionSettings pos_set_finger_index_proximal =  {-1.0e6f, 1.0e6f,  25.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//  S5FHPositionSettings pos_set_finger_middle_distal =   {-1.0e6f, 1.0e6f,  16.917e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//  S5FHPositionSettings pos_set_finger_middle_proximal = {-1.0e6f, 1.0e6f,  25.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//  S5FHPositionSettings pos_set_finger_ring =            {-1.0e6f, 1.0e6f,  22.959e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//  S5FHPositionSettings pos_set_finger_pinky =           {-1.0e6f, 1.0e6f,  22.959e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//  S5FHPositionSettings pos_set_spread =                 {-1.0e6f, 1.0e6f, 21.551e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};

  // All Fingers with a speed that will close the complete range of the finger in 1 Seconds    (except the thumb that wikll take 4) -> Changed that... lets see whats happening
    S5FHPositionSettings pos_set_thumb_flexion =          {-1.0e6f, 1.0e6f,  65.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
    S5FHPositionSettings pos_set_thumb_opposition =       {-1.0e6f, 1.0e6f,  50.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
    S5FHPositionSettings pos_set_finger_index_distal =    {-1.0e6f, 1.0e6f,  45.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
    S5FHPositionSettings pos_set_finger_index_proximal =  {-1.0e6f, 1.0e6f,  40.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
    S5FHPositionSettings pos_set_finger_middle_distal =   {-1.0e6f, 1.0e6f,  45.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
    S5FHPositionSettings pos_set_finger_middle_proximal = {-1.0e6f, 1.0e6f,  40.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
    S5FHPositionSettings pos_set_finger_ring =            {-1.0e6f, 1.0e6f,  45.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
    S5FHPositionSettings pos_set_finger_pinky =           {-1.0e6f, 1.0e6f,  45.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
    S5FHPositionSettings pos_set_spread =                 {-1.0e6f, 1.0e6f,  25.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};

    // All Fingers with a speed that will close the complete range of the finger in 0.5 Seconds (except the thumb that will take 4)
//    S5FHPositionSettings pos_set_thumb_flexion =          {-1.0e6f, 1.0e6f,  42.5e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//    S5FHPositionSettings pos_set_thumb_opposition =       {-1.0e6f, 1.0e6f,  25.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//    S5FHPositionSettings pos_set_finger_index_distal =    {-1.0e6f, 1.0e6f,  90.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//    S5FHPositionSettings pos_set_finger_index_proximal =  {-1.0e6f, 1.0e6f,  80.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//    S5FHPositionSettings pos_set_finger_middle_distal =   {-1.0e6f, 1.0e6f,  90.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//    S5FHPositionSettings pos_set_finger_middle_proximal = {-1.0e6f, 1.0e6f,  80.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//    S5FHPositionSettings pos_set_finger_ring =            {-1.0e6f, 1.0e6f,  90.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//    S5FHPositionSettings pos_set_finger_pinky =           {-1.0e6f, 1.0e6f,  90.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//    S5FHPositionSettings pos_set_spread =                 {-1.0e6f, 1.0e6f,  50.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};



  default_position_settings[0] = pos_set_thumb_flexion;   // thumb flexion
  default_position_settings[1] = pos_set_thumb_opposition;   // thumb opposition
  default_position_settings[2] = pos_set_finger_index_distal;  // index finger distal joint
  default_position_settings[3] = pos_set_finger_index_proximal;  // index finger proximal joint
  default_position_settings[4] = pos_set_finger_middle_distal;  // middle finger distal joint
  default_position_settings[5] = pos_set_finger_middle_proximal;  // middle finger proximal joint
  default_position_settings[6] = pos_set_finger_ring;  // ring finger
  default_position_settings[7] = pos_set_finger_pinky;  // pinky
  default_position_settings[8] = pos_set_spread;  // finger spread

  return default_position_settings;
}

// Converts joint positions of a specific channel from RAD to ticks
int32_t S5FHFingerManager::convertRad2Ticks(const S5FHCHANNEL &channel, double position)
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

// Check bounds of target positions
bool S5FHFingerManager::isInsideBounds(const S5FHCHANNEL &channel, const int32_t &target_position)
{
  return target_position >= m_position_min[channel] && target_position <= m_position_max[channel];
}

bool S5FHFingerManager::readParametersFromConfigFile()
{
//  bool read_successful = false;

//  // load position settings from config file
//  std::vector<S5FHPositionSettings> position_config_list;
//  read_successful =
//    icc::get(CONFIG_VALUES(
//               CONFIG_LIST(
//                 S5FHPositionSettings, "/S5FH/PositionSettings",
//                 MEMBER_MAPPING(
//                   S5FHPositionSettings,
//                   MEMBER_VALUE_1("WMin", S5FHPositionSettings, wmn)
//                   MEMBER_VALUE_1("WMax", S5FHPositionSettings, wmx)
//                   MEMBER_VALUE_1("DWMax", S5FHPositionSettings, dwmx)
//                   MEMBER_VALUE_1("KY", S5FHPositionSettings, ky)
//                   MEMBER_VALUE_1("DT", S5FHPositionSettings, dt)
//                   MEMBER_VALUE_1("IMin", S5FHPositionSettings, imn)
//                   MEMBER_VALUE_1("IMax", S5FHPositionSettings, imx)
//                   MEMBER_VALUE_1("KP", S5FHPositionSettings, kp)
//                   MEMBER_VALUE_1("KI", S5FHPositionSettings, ki)
//                   MEMBER_VALUE_1("KD", S5FHPositionSettings, kd)),
//                 std::back_inserter(position_config_list))),
//             DriverS5FH::instance());

//  // set controller position settings
//  if (read_successful)
//  {
//    for (size_t i = 0; i < position_config_list.size(); i++)
//    {
//      m_controller->setPositionSettings(i, position_config_list[i]);

//      LOGGING_ERROR_C(DriverS5FH, S5FHController, "new position settings recieved: " << endl <<
//                      "WMin = " << position_config_list[i].wmn << endl);
//    }
//  }
//  else
//  {
//    LOGGING_ERROR_C(DriverS5FH, S5FHFingerManager, "Could not load position settings from config file" << endl);
//  }

//  // load current settings from config file
//  std::vector<S5FHCurrentSettings> current_config_list;
//  read_successful =
//    icc::get(CONFIG_VALUES(
//               CONFIG_LIST(
//                 S5FHCurrentSettings, "/S5FH/CurrentSettings",
//                 MEMBER_MAPPING(
//                   S5FHCurrentSettings,
//                   MEMBER_VALUE_1("WMin", S5FHCurrentSettings, wmn)
//                   MEMBER_VALUE_1("WMax", S5FHCurrentSettings, wmx)
//                   MEMBER_VALUE_1("KY", S5FHCurrentSettings, ky)
//                   MEMBER_VALUE_1("DT", S5FHCurrentSettings, dt)
//                   MEMBER_VALUE_1("IMin", S5FHCurrentSettings, imn)
//                   MEMBER_VALUE_1("IMax", S5FHCurrentSettings, imx)
//                   MEMBER_VALUE_1("KP", S5FHCurrentSettings, kp)
//                   MEMBER_VALUE_1("KI", S5FHCurrentSettings, ki)
//                   MEMBER_VALUE_1("UMin", S5FHCurrentSettings, umn)
//                   MEMBER_VALUE_1("UMax", S5FHCurrentSettings, umx)),
//                 std::back_inserter(current_config_list))),
//             icl_core::logging::Nirwana::instance());

//  // set current position settings
//  if (read_successful)
//  {
//    for (size_t i = 0; i < current_config_list.size(); i++)
//    {
//      m_controller->setCurrentSettings(i, current_config_list[i]);
//    }
//  }
//  else
//  {
//    LOGGING_ERROR_C(DriverS5FH, S5FHFingerManager, "Could not load current settings from config file" << endl);
//  }
  return true;
}

}
