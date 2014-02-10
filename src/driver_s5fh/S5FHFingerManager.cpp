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
}

S5FHFingerManager::~S5FHFingerManager()
{
}

}
