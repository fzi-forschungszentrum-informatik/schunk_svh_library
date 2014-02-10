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
#ifndef S5FHCONTROLCOMMAND_H
#define S5FHCONTROLCOMMAND_H

#include <icl_core/BaseTypes.h>
#include <icl_comm/ByteOrderConversion.h>

namespace driver_s5fh {

//! ControlCommands are given as a single target position for the position controller (given in ticks)
typedef S5FHControlCommand uint32_t;


//! overload stream operator to easily serialize data
//! slightly uneccessary at this point but put in anayway to make sure it is used :)
inline icl_comm::ArrayBuilder& operator << (icl_comm::ArrayBuilder& ab, const S5FHControlCommand& data)
{
  return ab << data;
}


}

#endif // S5FHCONTROLCOMMAND_H
