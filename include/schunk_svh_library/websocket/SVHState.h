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

#ifndef SVHSTATE_H
#define SVHSTATE_H

#include <schunk_svh_library/ImportExport.h>
#include <schunk_svh_library/websocket/RobotState.h>

namespace schunk_svh_library {
namespace websocket {

class DRIVER_SVH_IMPORT_EXPORT SVHState : public RobotState
{
public:
  enum MovementState
  {
    eST_DEACTIVATED,
    eST_RESETTING,
    eST_RESETTED,
    eST_ENABLED,
    eST_PARTIALLY_ENABLED,
    eST_FAULT,
    eST_DIMENSION
  };


  SVHState();

  /*!
   * \brief getStateJSON Returns the current robot state as JSON encoded String
   * \return current robot state as JSON encoded String
   */
  virtual std::string getStateJSON();

  virtual void setMovementState(const int& movement_state);

private:
  //! Current movement state of the whole robot
  MovementState m_movement_state;

  //! Description values to get the corresponding enum value to a channel
  static const std::string m_joint_descriptions[];
};


} // namespace websocket
} // namespace schunk_svh_library

#endif // SVHSTATE_H
