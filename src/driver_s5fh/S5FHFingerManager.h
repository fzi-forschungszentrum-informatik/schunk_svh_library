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
#ifndef DRIVER_S5FH_S5FH_FINGER_MANAGER_H_INCLUDED
#define DRIVER_S5FH_S5FH_FINGER_MANAGER_H_INCLUDED

#include <driver_s5fh/ImportExport.h>
#include <driver_s5fh/S5FHController.h>
#include <driver_s5fh/S5FHFeedbackPollingThread.h>

//#ifdef _IC_BUILDER_ICL_COMM_WEBSOCKET_
// Shunk Diagnostics addon
#include <icl_comm_websocket/WsBroadcaster.h>
#include <boost/shared_ptr.hpp>
//#endif // _IC_BUILDER_ICL_COMM_WEBSOCKET_

namespace driver_s5fh {

/*! This class manages controller parameters and the finger reset.
 */
class DRIVER_S5FH_IMPORT_EXPORT S5FHFingerManager
{
public:

  /*! Constructs a finger manager for the SCHUNK five finger hand.
   * \param autostart if set to true, the driver will immediately connect to the hardware and try to reset all fingers
   * \param dev_name the dev to use for autostart. Default is /dev/ttyUSB0
   */
  S5FHFingerManager(const bool &autostart = false, const std::string &dev_name = "/dev/ttyUSB0");

  virtual ~S5FHFingerManager();

  //!
  //! \brief Open connection to SCHUNK five finger hand. Wait until expected return packages are received.
  //! \param dev_name
  //! \return
  //!
  bool connect(const std::string &dev_name);

  //!
  //! \brief disconnect SCHUNK five finger hand
  //!
  void disconnect();

  //!
  //! \brief returns connected state of finger manager
  //! \return bool
  //!
  bool isConnected() { return m_connected; }

  //!
  //! \brief reset function for channel
  //! \param channel
  //! \return
  //!
  bool resetChannel(const S5FHCHANNEL &channel);

  //!
  //! \brief enable controller of channel
  //! \param channel
  //! \return
  //!
  bool enableChannel(const S5FHCHANNEL &channel);

  //!
  //! \brief disable controller of channel
  //! \param channel
  //! \return
  //!
  void disableChannel(const S5FHCHANNEL &channel);

  //!
  //! \brief sends request controller feedback packet for all channels
  //! \return
  //!
  bool requestControllerFeedbackAllChannels();

  //!
  //! \brief send request controller feedback paket
  //! \param channel
  //! \return
  //!
  bool requestControllerFeedback(const S5FHCHANNEL &channel);

  //!
  //! \brief returns position value of channel
  //! \param channel
  //! \param position
  //! \return bool
  //!
  bool getPosition(const S5FHCHANNEL &channel, double &position);

  //!
  //! \brief returns current value of channel
  //! \param channel
  //! \param current
  //! \return bool
  //!
  bool getCurrent(const S5FHCHANNEL &channel, double &current);

  //!
  //! \brief returns actual current controller settings of channel
  //! \param channel
  //! \param current_settings
  //! \return
  //!
  bool getCurrentControllerParams(const S5FHCHANNEL &channel, S5FHCurrentSettings &current_settings);

  //!
  //! \brief returns actual position controller settings of channel
  //! \param channel
  //! \param position_settings
  //! \return
  //!
  bool getPositionControllerParams(const S5FHCHANNEL &channel, S5FHPositionSettings &position_settings);

  //!
  //! \brief set all target positions at once
  //! \param positions
  //! \return
  //!
  bool setAllTargetPositions(const std::vector<double>& positions);

  //!
  //! \brief set target position of a channel
  //! \param channel
  //! \param position
  //! \param current
  //! \return
  //!
  bool setTargetPosition(const S5FHCHANNEL &channel, double position, double current);

  //!
  //! \brief overwrite current parameters
  //! \param channel
  //! \param current_settings
  //! \return
  //!
  bool setCurrentControllerParams(const S5FHCHANNEL &channel, const S5FHCurrentSettings &current_settings);

  //!
  //! \brief overwrite position parameters
  //! \param channel
  //! \param position_settings
  //! \return
  //!
  bool setPositionControllerParams(const S5FHCHANNEL &channel, const S5FHPositionSettings &position_settings);

  //!
  //! \brief returns true, if current channel has been enabled
  //! \param channel
  //! \return
  //!
  bool isEnabled(const S5FHCHANNEL &channel);

  //!
  //! \brief returns true, if current channel has been resetted
  //! \param channel
  //! \return
  //!
  bool isHomed(const S5FHCHANNEL &channel);

// ----------------------------------------------------------------------
// ---- private functions and varaibles
// ----------------------------------------------------------------------

private:

  //#ifdef _IC_BUILDER_ICL_COMM_WEBSOCKET_
    boost::shared_ptr<icl_comm::websocket::WsBroadcaster> ws_broadcaster;
  //#endif // _IC_BUILDER_ICL_COMM_WEBSOCKET_

  //! \brief pointer to s5fh controller
  S5FHController *m_controller;

  //! \brief pointer to s5fh controller
  S5FHFeedbackPollingThread *m_feedback_thread;

  //! \brief holds the connected state
  bool m_connected;

  //! \brief vector storing reset flags for each finger
  int8_t m_homing_timeout;

  //! data sctructure for home positions
  struct
  {
    int   direction;         // +1 or -1 : home in positive or negative direction
    float minimumOffset;     // offset from home position to minimum (soft limit)
    float maximumOffset;     // offset from home position to maximum (soft limit)
    float idlePosition;      // position to go to after intialization
  } typedef HomeSettings;

  //! \brief home position default settings vector for each channel
  std::vector<HomeSettings> m_home_settings;

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

  //! \brief vector storing the reset order of the channels
  std::vector<S5FHCHANNEL> m_reset_order;

  /*!
    * \brief Vector containing factors for the currents at reset.
    * Vector containing factors for the currents at reset.
    * A hard stop is found if the maxCurrent (first 2 CurrentSettingsValues) x the reset factor was reached. 0.75 by default
    * Beware. Setting this value very high might result in damage to the motors during reset.
    */
  std::vector<double> m_reset_current_factor;

  //! \brief set default parameters for home position
  void setHomePositionDefaultParameters();

  //! \brief get default parameters for current settings
  std::vector<S5FHCurrentSettings> getCurrentSettingsDefaultParameters();

  //! \brief get default parameters for position settings
  std::vector<S5FHPositionSettings> getPositionSettingsDefaultParameters();

  //! \brief get default parameters for position settings DURING RESET
  std::vector<S5FHPositionSettings> getPositionSettingsDefaultResetParameters();

  //!
  //! \brief Converts joint positions of a specific channel from RAD to ticks
  //! \param channel
  //! \param position
  //! \return
  //!
  int32_t convertRad2Ticks(const S5FHCHANNEL &channel, double position);

  //!
  //! \brief Check bounds of target positions
  //! \param channel
  //! \param target_position
  //! \return
  //!
  bool isInsideBounds(const S5FHCHANNEL &channel, const int32_t &target_position);

  //!
  //! \brief readParametersFromConfigFile
  //! \return
  //!
  bool readParametersFromConfigFile();


};

}

#endif
