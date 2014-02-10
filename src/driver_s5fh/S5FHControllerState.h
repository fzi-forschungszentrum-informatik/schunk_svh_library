// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Georg Heppner
 * \date    2014-02-03
 *
 */
//----------------------------------------------------------------------
#ifndef S5FHCONTROLLERSTATE_H
#define S5FHCONTROLLERSTATE_H

#include <icl_core/BaseTypes.h>

namespace driver_s5fh {

/*!
 * \brief The S5FHControllerState indicates the current State of the MeCoVis controllerm IC which is used in the S5FH
 */
struct S5FHControllerState
{
  //! Reset of controller??? (0x001F to Reset)
  uint16_t pwm_fault;
  //! Reset of controller??? (0x001F to Reset)
  uint16_t pwm_otw;
  //! Enable 12V supply driver??? (0x0200 to activate)
  uint16_t pwm_reset;
  //! Enable 12V supply driver??? (0x0200 to activate)
  uint16_t pwm_active;
  //! Enable/Disable of position controller (0x0001 to Activate)
  uint16_t pos_ctrl;
  //! Enable/Disbale of current controller (0x0001 to Activate)
  uint16_t cur_ctrl;
};
//! overload stream operator to easily serialize data
inline icl_comm::ArrayBuilder& operator << (icl_comm::ArrayBuilder& ab, const S5FHControllerState& data)
{
  ab << data.pwm_fault
     << data.pwm_otw
     << data.pwm_reset
     << data.pwm_active
     << data.pos_ctrl
     << data.cur_ctrl;
  return ab;
}


}


#endif // S5FHCONTROLLERSTATE_H
