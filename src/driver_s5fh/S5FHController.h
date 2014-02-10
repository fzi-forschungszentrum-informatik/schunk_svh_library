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
#ifndef DRIVER_S5FH_S5FH_CONTROLLER_H_INCLUDED
#define DRIVER_S5FH_S5FH_CONTROLLER_H_INCLUDED

#include "driver_s5fh/ImportExport.h"
#include "driver_s5fh/S5FHSerialInterface.h"
#include "driver_s5fh/S5FHControlCommand.h"
#include "driver_s5fh/S5FHControllerFeedback.h"
#include "driver_s5fh/S5FHCurrentSettings.h"
#include "driver_s5fh/S5FHFirmwareInfo.h"
#include "driver_s5fh/S5FHPositionSettings.h"

namespace driver_s5fh {

/*! This class controls the the SCHUNK five finger hand.
 */
class DRIVER_S5FH_IMPORT_EXPORT S5FHController
{
public:
  /*! Constructs a controller class for the SCHUNK five finger hand.
   */
  S5FHController();

  ~S5FHController();

  //! overwrite current controller parameters of finger index
  void setCurrentSettings(size_t index, const S5FHCurrentSettings& current_settings)
  {
    m_current_settings[index] = current_settings;
  }

  //! overwrite position controller parameters of finger index
  void setPositionSettings(size_t index, const S5FHPositionSettings& position_settings)
  {
    m_position_settings[index] = position_settings;
  }

  //! setting new position controller target for finger index
  void setControllerTarget(size_t index, const S5FHControlCommand& target_position);

private:

  //! vector of current controller parameters for each finger
  std::vector<S5FHCurrentSettings> m_current_settings;

  //! vector of position controller parameters for each finger
  std::vector<S5FHPositionSettings> m_position_settings;

};

}

#endif
