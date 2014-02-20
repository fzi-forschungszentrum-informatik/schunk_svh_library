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

namespace driver_s5fh {

/*! This class manages controller parameters and the finger reset.
 */
class DRIVER_S5FH_IMPORT_EXPORT S5FHFingerManager
{
public:

  /*! Constructs a finger manager for the SCHUNK five finger hand.
   */
  S5FHFingerManager();

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
  //! \return
  //!
  bool setCurrentControllerParams(const S5FHCHANNEL &channel, const S5FHCurrentSettings &current_settings);

  //!
  //! \brief overwrite position parameters
  //! \param channel
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

  //! \brief set default parameters for home position
  void setHomePositionDefaultParameters();

  //! \brief get default parameters for current settings
  std::vector<S5FHCurrentSettings> getCurrentSettingsDefaultParameters();

  //! \brief get default parameters for position settings
  std::vector<S5FHPositionSettings> getPositionSettingsDefaultParameters();

  //! \brief get default parameters for position settings DURING RESET
  std::vector<S5FHPositionSettings> getPositionSettingsDefaultResetParameters();

  //!
  //! \brief readParametersFromConfigFile
  //! \return
  //!
  bool readParametersFromConfigFile();


};

}

#endif
