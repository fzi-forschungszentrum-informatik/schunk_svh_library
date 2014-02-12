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

  S5FHControllerState(uint16_t _pwm_fault =0,uint16_t _pwm_otw = 0 , uint16_t _pwm_reset = 0 ,uint16_t _pwm_active = 0 ,uint16_t _pos_ctrl =0, uint16_t _cur_ctrl =0):
  pwm_fault(_pwm_fault),
  pwm_otw(_pwm_otw),
  pwm_reset(_pwm_reset),
  pwm_active(_pwm_active),
  pos_ctrl(_pos_ctrl),
  cur_ctrl(_cur_ctrl)
  {  }

    //! Compares two S5FHControllerState objects.
  bool operator == (const S5FHControllerState& other) const
  {
    return
      (pwm_fault == other.pwm_fault
       && pwm_otw == other.pwm_otw
       && pwm_reset == other.pwm_reset
       && pwm_active == other.pwm_active
       && pos_ctrl == other.pos_ctrl
       && cur_ctrl == other.cur_ctrl);
  }
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

//! overload stream operator to easily serialize data
inline icl_comm::ArrayBuilder& operator >> (icl_comm::ArrayBuilder& ab, S5FHControllerState& data)
{
  // Dirty? Hack.. if the Arraybuilder is longer then the things we expect we just overwrite that..
  // This will be a problem if for example you would want to write ab >> data_struct >>data_struct
  // We will think of something better later.. for now we just expect to be a packet the size we want it to be
  if (ab.pos > 12)
  {
    ab.pos = 12;
  }
  ab >> data.cur_ctrl
     >> data.pos_ctrl
     >> data.pwm_active
     >> data.pwm_reset
     >> data.pwm_otw
     >> data.pwm_fault;
  return ab;
}

//! Output Stream operator
inline std::ostream& operator << (std::ostream& o, const S5FHControllerState& cs)
{
  o << std::setw(4) << std::setfill('0') << std::hex
    << "pwm_fault " << "0x" << static_cast<int>(cs.pwm_fault) << " "
    << "pwm_otw " << "0x" << static_cast<int>(cs.pwm_otw) << " "
    << "pwm_reset " << "0x" << static_cast<int>(cs.pwm_reset) << " "
    << "pwm_active " << "0x" << static_cast<int>(cs.pwm_active) << " "
    << "pos_ctr " << "0x" <<  static_cast<int>(cs.pos_ctrl) << " "
    << "cur_ctrl " << "0x" << static_cast<int>(cs.cur_ctrl) << " "
    << std::endl;
  // Reset Output stream to decimal output .. otherwise it may confuse people
  std::cout << std::dec ;
  return o;
}


}


#endif // S5FHCONTROLLERSTATE_H
