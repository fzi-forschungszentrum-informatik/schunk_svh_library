// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-


// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Georg Heppner <heppner@fzi.de>
 * \date    2014-5-22
 *
 */
//----------------------------------------------------------------------

#ifndef LWA4PSTATE_H
#define LWA4PSTATE_H

#include <schunk_svh_library/ImportExport.h>
#include <schunk_svh_library/websocket/RobotState.h>

namespace schunk_svh_library {
namespace websocket {

class DRIVER_SVH_IMPORT_EXPORT LWA4PState : public RobotState
{
public:
  enum MovementState
  {
    eST_DEACTIVATED,
    eST_CONTROLLED,
    eST_FAULT,
    eST_CHAIN_BUILT_UP,
    eST_IPM_MODE,
    eST_ERROR_ACKED,
    eST_COMMUTATION_FOUND,
    eST_PSEUDE_ABSOLUT_CALIBRATED,
    eST_JITTER_SYNCED,
    eST_SWITCH_OF_POSE_DIFFERS,
    eST_SWITCH_OF_POSE_VERIFIED,
    eST_READY,
    eST_ENABLED,
    eST_ABSOLUTE_POSITION_LOST,
    eST_DIMENSION
  };

  LWA4PState();


  /*!
   * \brief getStateJSON Returns the current robot state as JSON encoded String
   * \return current robot state as JSON encoded String
   */
  virtual std::string getStateJSON();

  virtual void setMovementState(const int& movement_state);

private:
  //! Current movement state of the whole robot
  MovementState m_movement_state;
};

} // namespace websocket
} // namespace schunk_svh_library

#endif // LWA4PSTATE_H
