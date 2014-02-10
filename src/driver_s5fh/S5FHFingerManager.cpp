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
#include "driver_s5fh/S5FHFingerManager.h"

#include "driver_s5fh/Logging.h"


namespace driver_s5fh {

S5FHFingerManager::S5FHFingerManager()
{
  m_controller = new S5FHController();

  // 1. load current and position setting from config file
  // 2. set controller with current and position setting

}

S5FHFingerManager::~S5FHFingerManager()
{
}

//! reset function for a single finger
bool S5FHFingerManager::resetFinger(const S5FHDOF &index)
{
  return true;
}

//! set target position of a single finger
bool S5FHFingerManager::setTargetPosition(const S5FHDOF &index, double position, double current)
{
  S5FHControlCommand position_command;
  position_command.position = position;

  m_controller->setControllerTarget(index, position_command);
  return true;
}

//! overwrite current parameters
bool S5FHFingerManager::setCurrentControllerParams(const S5FHDOF &index)
{
  S5FHCurrentSettings current_settings;
  // TODO: set parameters

  m_controller->setCurrentSettings(index, current_settings);
  return true;
}

//! overwrite position parameters
bool S5FHFingerManager::setPositionControllerParams(const S5FHDOF &index)
{
  S5FHPositionSettings position_settings;
  // TODO: set parameters

  m_controller->setPositionSettings(index, position_settings);
  return true;
}

}
