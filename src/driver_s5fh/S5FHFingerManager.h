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

#include "driver_s5fh/ImportExport.h"
#include "driver_s5fh/S5FHController.h"

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
  //! \brief open connection to SCHUNK five finger hand
  //! \param dev_name
  //! \return
  //!
  bool connect(const std::string &dev_name);

  //!
  //! \brief disconnect SCHUNK five finger hand
  //!
  void disconnect();

  //!
  //! \brief reset function for a single finger
  //! \param index
  //! \return
  //!
  bool resetChannel(const S5FHCHANNEL &channel);

  //!
  //! \brief set target position of a single finger
  //! \param index
  //! \param position
  //! \param current
  //! \return
  //!
  bool setTargetPosition(const S5FHCHANNEL &channel, double position, double current);

  //!
  //! \brief overwrite current parameters
  //! \param index
  //! \return
  //!
  bool setCurrentControllerParams(const S5FHCHANNEL &channel, const S5FHCurrentSettings &current_settings);

  //!
  //! \brief overwrite position parameters
  //! \param index
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

private:

  //!
  //! \brief pointer to s5fh controller
  //!
  S5FHController *m_controller;

  //! data sctructure for home positions
  struct
  {
    int   direction;         // +1 or -1 : home in positive or negative direction
    float minimumOffset;     // offset from home position to minimum (soft limit)
    float maximumOffset;     // offset from home position to maximum (soft limit)
    float idlePosition;      // position to go to after intialization
  } typedef HomeSettings;

  //!
  //! \brief home position default settings vector for each finger
  //!
  std::vector<HomeSettings> m_home_settings;

  //!
  //! \brief min position vector for each finger
  //!
  std::vector<u_int32_t> m_position_min;

  //!
  //! \brief max position vector for each finger
  //!
  std::vector<u_int32_t> m_position_max;

  //!
  //! \brief vector storing reset flags for each finger
  //!
  std::vector<bool> m_is_homed;

  //!
  //! \brief set default parameters for home position
  //!
  void setHomePositionDefaultParameters();

  //!
  //! \brief get default parameters for current settings
  //!
  std::vector<S5FHCurrentSettings> getCurrentSettingsDefaultParameters();

  //!
  //! \brief get default parameters for position settings
  //!
  std::vector<S5FHPositionSettings> getPositionSettingsDefaultParameters();

  //!
  //! \brief readParametersFromConfigFile
  //! \return
  //!
  bool readParametersFromConfigFile();

};

}

#endif
