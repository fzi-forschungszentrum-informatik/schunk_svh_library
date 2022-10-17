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
 * \author  Lars pfotzer
 * \date    2014-01-30
 *
 * This file contains the Finger Manager
 * that is managing the Schunk five finger hand on a high level.
 * The fingermanager is responsible to filter all calls and only make
 * allowed calls to the controller. The fingermanager is also responsible
 * for storing any kind of Konfiguration (like current controller settings).
 * The fingermanager is also responsible to poll the controller for continious data (if wanted)
 */
//----------------------------------------------------------------------
#ifndef DRIVER_SVH_SVH_FINGER_MANAGER_H_INCLUDED
#define DRIVER_SVH_SVH_FINGER_MANAGER_H_INCLUDED

#include <chrono>
#include <schunk_svh_library/ImportExport.h>
#include <schunk_svh_library/control/SVHController.h>
#include <schunk_svh_library/control/SVHCurrentSettings.h>
#include <schunk_svh_library/control/SVHHomeSettings.h>
#include <schunk_svh_library/control/SVHPositionSettings.h>

#include <memory>
#include <thread>


namespace driver_svh {

/*! This class manages controller parameters and the finger reset.
 */
class DRIVER_SVH_IMPORT_EXPORT SVHFingerManager
{
public:
  /*!
   * \brief The Hints enum provides mapping to hints that can be sent to the web-diagnostic
   * interface
   */
  enum Hints
  {
    HT_DEVICE_NOT_FOUND,    /* ttyUSBx could not be found */
    HT_CONNECTION_FAILED,   /* ttyUSB could be opened but communication failed */
    HT_NOT_RESETTED,        /* the fingers of the hand are not resetted */
    HT_NOT_CONNECTED,       /* simply never called connect */
    HT_RESET_FAILED,        /* timeout during reset -> this is a serious failure */
    HT_CHANNEL_SWITCHED_OF, /* Not realy a problem, however a hint worth noting */
    HT_DANGEROUS_CURRENTS,  /* Current Values are set to dangerous levels */
    HT_DIMENSION            /* dummy entry indicating the size, not used as status */
  };

  struct DiagnosticState
  {
    bool diagnostic_encoder_state;
    bool diagnostic_motor_state;
    double diagnostic_current_maximum;
    double diagnostic_current_minimum;
    double diagnostic_position_maximum;
    double diagnostic_position_minimum;
    double diagnostic_deadlock;
  };

  /*! Constructs a finger manager for the SCHUNK five finger hand.
   * \param autostart if set to true, the driver will immediately connect to the hardware and try to
   * reset all fingers \param dev_name the dev to use for autostart. Default is /dev/ttyUSB0
   */
  SVHFingerManager(const std::vector<bool>& disable_mask = std::vector<bool>(9, false),
                   const uint32_t& reset_timeout         = 5);

  virtual ~SVHFingerManager();

  /*!
   *  \brief Open connection to SCHUNK five finger hand. Wait until expected return packages are
   * received. \param dev_name file handle of the serial device e.g. "/dev/ttyUSB0" \param
   * _retry_count The number of times a connection is tried to be established if at least one
   * package was received \return true if connection was succesful
   */
  bool connect(const std::string& dev_name = "/dev/ttyUSB0", const unsigned int& retry_count = 3);

  //!
  //! \brief disconnect SCHUNK five finger hand
  //!
  void disconnect();

  //!
  //! \brief returns connected state of finger manager
  //! \return bool true if the finger manager is connected to the hardware
  //!
  bool isConnected() { return m_connected; }

  //!
  //! \brief reset function for channel
  //! \param channel Channel to reset
  //! \return true if the reset was successful
  //!
  bool resetChannel(const SVHChannel& channel);

  //!
  //! \brief enable controller of channel
  //! \param channel channel to enable
  //! \return true if the enabling was successful
  //!
  bool enableChannel(const SVHChannel& channel);

  //!
  //! \brief disable controller of channel
  //! \param channel channel to disable
  //!
  void disableChannel(const SVHChannel& channel);

  //!
  //! \brief sends request controller feedback packet for all channels
  //! \return true if the request was successfully send to the hardware
  //!
  bool requestControllerFeedbackAllChannels();

  //!
  //! \brief send request controller feedback paket
  //! \param channel channel to request the feedback for
  //! \return true if the request was successfully send to the hardware
  //!
  bool requestControllerFeedback(const SVHChannel& channel);

  //!
  //! \brief returns position value of channel (will not acces hardware but return latest value)
  //! \param channel channel to get the position of
  //! \param position position the given channel ist at
  //! \return bool true if a valid result was requested (i.e. an existing channel)
  //!
  bool getPosition(const SVHChannel& channel, double& position);

  //!
  //! \brief returns current value of channel
  //! \param channel channel to get the current of
  //! \param current current of the given channel in [mA]
  //! \return bool true if a valid result was requested (i.e. an existing channel)
  //!
  bool getCurrent(const SVHChannel& channel, double& current);


  //!
  //! \brief set all target positions at once
  //! \param positions Vector of positions to set as targets given in [rad]. Only the first
  //! eSVH_CHANNEL_DIMENSION (9) values will be considered. If less values are given all others are
  //! set to zero \return true if a valid and wellformed Target position for all fingers was given
  //! and send to the HW (i.e. inside the bound limits etc.)
  //!
  bool setAllTargetPositions(const std::vector<double>& positions);

  //!
  //! \brief set target position of a channel
  //! \param channel channel to set the target position for
  //! \param position target position in [rad]
  //! \param current max current to use for that position @note CURRENTLY NOT SUPPORTED!! WILL BE
  //! IGNORED \return true if a valid target position was given and it could be sent to the hardware
  //!
  bool setTargetPosition(const SVHChannel& channel, double position, double current);

  //!
  //! \brief returns true, if current channel has been enabled
  //! \param channel channel to check if it is enabled
  //! \return true if the channel is enabled, false otherwise
  //!
  bool isEnabled(const SVHChannel& channel);

  //!
  //! \brief returns true, if current channel has been resetted
  //! \param channel
  //! \return
  //!
  bool isHomed(const SVHChannel& channel);

  //! requests the current controller state to be updated
  //! @note This is a debuging function. Should not be called by users
  void requestControllerState();

  //!
  //! \brief returns actual current controller settings of channel
  //! \param channel channel to get the current controller settings for
  //! \param current_settings settings currently active for the current controller
  //! \return true if a valid result was requested (i.e. an existing channel)
  //!
  bool getCurrentSettings(const SVHChannel& channel, SVHCurrentSettings& current_settings);

  //!
  //! \brief returns actual position controller settings of channel
  //! \param channel channel to get the position settings for
  //! \param position_settings settings currently active for the position controller
  //! \return true if a valid result was requested (i.e. an existing channel)
  //!
  bool getPositionSettings(const SVHChannel& channel, SVHPositionSettings& position_settings);

  //!
  //! \brief returns actual home settings of channel
  //! \param channel channel to get the position settings for
  //! \param home_settings settings indicating the movement range of a finger, its reset direction
  //! and idle position \return true if a valid result was requested (i.e. an existing channel)
  //!
  bool getHomeSettings(const SVHChannel& channel, SVHHomeSettings& home_settings);

  //!
  //! \brief returns actual diagnostic status of channel
  //! \param channel channel to get the position settings for
  //! \param diagnostic_status diagnostic data of motor and encoder
  //! \return true if a valid result was requested (i.e. an existing channel)
  //!
  bool getDiagnosticStatus(const SVHChannel& channel, struct DiagnosticState& diagnostic_status);

  //!
  //! \brief overwrite current parameters
  //! \param channel channel to set the current settings for
  //! \param current_settings settings of the current controller for a specific channel
  //! \return true if a valid channel was selected
  //!
  bool setCurrentSettings(const SVHChannel& channel, const SVHCurrentSettings& current_settings);

  //!
  //! \brief overwrite position parameters
  //! \param channel channel to set the positoon settings for
  //! \param position_settings settings of the position controller to be used
  //! \return true if a valid channel was selected
  //!
  bool setPositionSettings(const SVHChannel& channel, const SVHPositionSettings& position_settings);

  //!
  //! \brief setHomeSettings set the home Settings which are maily used doring reset and provide the
  //! soft limit for the fingers \param channel channel to set the home settings for \param
  //! home_settings settings indicating the movement range of a finger, its reset direction and idle
  //! position \return true if a valid channel was selected
  //!
  bool setHomeSettings(const SVHChannel& channel, const SVHHomeSettings& home_settings);

  //!
  //! \brief resetDiagnosticData reset the diagnostic data vectors
  //! \param channel channel to reset the data vector for
  //! \return true if a valid channel was selected
  //!
  bool resetDiagnosticData(const SVHChannel& channel);


  // These 3 functions could be private but where made public for printing and debug purposes. As
  // there is no harm to it it should not be a problem

  //! \brief get default current settings. These are either values previously set from calling
  //! software or hardcoded defaults
  std::vector<SVHCurrentSettings> getDefaultCurrentSettings();

  //! \brief get default position settings. These are either values previously set from calling
  //! software or hardcoded defaults \parm reset true if the Positions settins are to be used during
  //! reset (reduced speed)
  std::vector<SVHPositionSettings> getDefaultPositionSettings(const bool& reset = false);

  //! \brief initialize the homing settings with hardcoded defaults. These can be overwritten by the
  //! setHomeSettings function
  void setDefaultHomeSettings();

  //!
  //! \brief setResetSpeed Set the speed percentage during reset
  //! \param speed percent of the normal speed used during reset Allowed values 0.0-1.0
  //!
  void setResetSpeed(const float& speed);

  //!
  //! \brief setResetTimeout Helper function to set the timout durind rest of fingers
  //! \param resetTimeout timeout in Seconds. Values smaler than 0 will be interpreted as 0
  //!
  void setResetTimeout(const int& reset_timeout);

  //!
  //! \brief setMaxForce set the max force / current as a persentage of the maximum possible current
  //! \param max_force in percent [0,1]
  //! return if valid max_force was given
  //!
  bool setMaxForce(float max_force);

  //!
  //! \brief setForceLimit set the force limit for the given channel
  //! \param channel channel to set the force limit for
  //! \param force_limit force limit to set
  //! \return value if ok, else 0.0
  //!
  float setForceLimit(const SVHChannel& channel, float force_limit);

  //!
  //! \brief Converts joint currents of a specific channel from current [mA] to force [N] factoring
  //! the effort_constants of the channels \param channel Channel to Convert for (each one has
  //! different constants) \param current Current in [mA] \return effort The desired effort in [N]
  //! (absolut)
  //!
  double convertmAtoN(const SVHChannel& channel, const int16_t& current);

  //!
  //! \brief getFirmwareInfo Requests the firmware information from the harware, waits a bit and
  //! returns the last one read. \note  if no connection is open and the param dev_name is given, a
  //! connection is opened and closed after the readout. \param dev_name file handle of the serial
  //! device e.g. "/dev/ttyUSB0" \param _retry_count The number of times a connection is tried to be
  //! established if at least one package was received \return the last firmware information read
  //! (this may not be the one currently requested)
  //!
  SVHFirmwareInfo getFirmwareInfo(const std::string& dev_name     = "/dev/ttyUSB0",
                                  const unsigned int& retry_count = 3);


  // ----------------------------------------------------------------------
  // ---- private functions and varaibles
  // ----------------------------------------------------------------------

private:
  //! \brief pointer to svh controller
  SVHController* m_controller;

  //! \brief Flag whether to poll feedback periodically in the feedback thread
  std::atomic<bool> m_poll_feedback;

  //! \brief Thread for polling periodic feedback from the hardware
  std::thread m_feedback_thread;

  //! \brief holds the connected state
  bool m_connected;

  //! Helper variable to check if feedback was printed (will be replaced by a better solution in the
  //! future)
  bool m_connection_feedback_given;

  //! \brief Timeout for homing.
  std::chrono::seconds m_homing_timeout;

  //! \brief limit the maximum of the force / current of the finger as a percentage of the possible
  //! maximum
  float m_max_current_percentage;

  //! \brief position conversion factor (ticks to RAD) for each channel
  std::vector<double> m_ticks2rad;

  //! \brief min position vector for each channel
  std::vector<int32_t> m_position_min;

  //! \brief max position vector for each channel
  std::vector<int32_t> m_position_max;

  //! \brief home position after complete reset of each channel
  std::vector<int32_t> m_position_home;

  //! \brief vector storing reset flags for each channel
  std::vector<bool> m_is_homed;

  //! vector storing information if a finger is enabled. In Case it is all request for it will be
  //! granted but not executed on hardware
  std::vector<bool> m_is_switched_off;

  //! \brief vectors storing diagnostic information, encoder state
  std::vector<bool> m_diagnostic_encoder_state;

  //! \brief vectors storing diagnostic information, current state
  std::vector<bool> m_diagnostic_current_state;

  //! \brief vectors storing diagnostic information, current maximum
  std::vector<double> m_diagnostic_current_maximum;

  //! \brief vectors storing diagnostic information, current minimum
  std::vector<double> m_diagnostic_current_minimum;

  //! \brief vectors storing diagnostic information, position maximum
  std::vector<double> m_diagnostic_position_maximum;

  //! \brief vectors storing diagnostic information, position minimum
  std::vector<double> m_diagnostic_position_minimum;

  //! \brief vectors storing diagnostic information, diagnostics deadlock
  std::vector<double> m_diagnostic_deadlock;

  //! Factor for determining the finger speed during reset. Only 0.0-1.0 is allowed
  float m_reset_speed_factor;

  //! Time in seconds after which the a reset is aborted if no change in current is observable
  std::chrono::seconds m_reset_timeout;

  //! Vector of current controller parameters for each finger (as given by external config)
  std::vector<SVHCurrentSettings> m_current_settings;

  //! Information about the validity of externaly given values for the current settings (easier to
  //! use this way)
  std::vector<bool> m_current_settings_given;

  //! Vector of position controller parameters for each finger (as given by external config)
  std::vector<SVHPositionSettings> m_position_settings;
  //! Information about the validity of externaly given values for the position settings (easier to
  //! use this way)
  std::vector<bool> m_position_settings_given;

  //! Vector of home settings for each finger (as given by external config)
  std::vector<SVHHomeSettings> m_home_settings;

  //! Firmware info of the connected Hand
  SVHFirmwareInfo m_firmware_info;

  /*!
   * \brief m_serial_device Device handle of the device to use, is overwritten if connect is called
   * with an argument
   */
  std::string m_serial_device;

  //! \brief vector storing the reset order of the channels
  std::vector<SVHChannel> m_reset_order;

  /*!
   * \brief Vector containing factors for the currents at reset.
   * Vector containing factors for the currents at reset.
   * A hard stop is found if the maxCurrent (first 2 CurrentSettingsValues) x the reset factor was
   * reached. 0.75 by default Beware. Setting this value very high might result in damage to the
   * motors during reset.
   */
  std::vector<double> m_reset_current_factor;

  //!
  //! \brief Converts joint positions of a specific channel from RAD to ticks factoring in the
  //! offset of the channels \param channel Channel to Convert for (each one has different offset)
  //! \param position The desired position given in RAD
  //! \return The tick value corresponing to the RAD input
  //!
  int32_t convertRad2Ticks(const SVHChannel& channel, const double& position);

  //!
  //! \brief Converts joint positions of a specific channel from ticks to RAD factoring in the
  //! offset of the channels \param channel Channel to Convert for (each one has different offset)
  //! \param ticks The current position in ticks
  //! \return the RAD Value corresponding to the tick value of a given channel
  //!
  double convertTicks2Rad(const SVHChannel& channel, const int32_t& ticks);

  //!
  //! \brief Converts joint efforts of a specific channel from force [N] to current [mA] factoring
  //! the effort_constants of the channels \param channel Channel to Convert for (each one has
  //! different constants) \param efforts Effort in [N] \return current The desired current in [mA]
  //!
  uint16_t convertNtomA(const SVHChannel& channel, const double& effort);

  //!
  //! \brief Check bounds of target positions
  //! \param channel
  //! \param target_position
  //! \return
  //!
  bool isInsideBounds(const SVHChannel& channel, const int32_t& target_position);

  /*!
   * \brief currentSettingsAreSafe helper function to check for the most important values of the
   * current settings \param channel the channel the settings are meant for \param current_settings
   * the settings to evaluate \return true if they are "reasonable safe". Only the most vile
   * settings will be rejected!
   */
  bool currentSettingsAreSafe(const SVHChannel& channel,
                              const SVHCurrentSettings& current_settings);

  /**
   * \brief Periodically poll feedback from the hardware
   *
   * The hardware will send data only in response to special requests. We realize constant joint
   * position feedback for all fingers through sending these requests periodically in this function.
   */
  void pollFeedback();

  // DEBUG
  SVHControllerFeedback m_debug_feedback;
};

} // namespace driver_svh

#endif
