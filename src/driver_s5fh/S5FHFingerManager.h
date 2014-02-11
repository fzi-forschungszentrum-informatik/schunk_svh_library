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

  enum {
    eS5FH_THUMB_FLEXION,
    eS5FH_THUMB_OPPOSITION, // wrist
    eS5FH_INDEX_FINGER_DISTAL,
    eS5FH_INDEX_FINGER_PROXIMAL,
    eS5FH_MIDDLE_FINGER_DISTAL,
    eS5FH_MIDDLE_FINGER_PROXIMAL,
    eS5FH_RING_FINGER,
    eS5FH_PINKY,
    eS5FH_FINGER_SPREAD,
    eS5FH_DIMENSION
  } typedef S5FHDOF;

  //!
  //! \brief reset function for a single finger
  //! \param index
  //! \return
  //!
  bool resetFinger(const S5FHDOF &index);

  //!
  //! \brief set target position of a single finger
  //! \param index
  //! \param position
  //! \param current
  //! \return
  //!
  bool setTargetPosition(const S5FHDOF &index, double position, double current);

  //!
  //! \brief overwrite current parameters
  //! \param index
  //! \return
  //!
  bool setCurrentControllerParams(const S5FHDOF &index);

  //!
  //! \brief overwrite position parameters
  //! \param index
  //! \return
  //!
  bool setPositionControllerParams(const S5FHDOF &index);

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
  //! \brief vector for home position default settings
  //!
  std::vector<HomeSettings> m_home_settings;

  //!
  //! \brief set default parameters for home position
  //!
  void setHomePositionDefaultParameters();

  //!
  //! \brief set default parameters for current settings
  //!
  void setCurrentSettingsDefaultParameters();

  //!
  //! \brief set default parameters for position settings
  //!
  void setPositionSettingsDefaultParameters();

  //!
  //! \brief readParametersFromConfigFile
  //! \return
  //!
  bool readParametersFromConfigFile();

};

}

#endif
