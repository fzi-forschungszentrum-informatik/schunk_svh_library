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

/*!
 * \brief ControlCommands are given as a single target position for the position controller (given in ticks)
 */
struct S5FHControlCommand
{
  //! Returned position value of the motor [Ticks]
  int32_t position;

  //!
  //! \brief Constructs a new control command to comandeer the position of the fingers
  //! \param _position target position given in encoder ticks
  //!
  S5FHControlCommand(const int32_t& _position):
    position(_position)
  {}

  //! Compares two S5FHControlCommand objects.
  bool operator == (const S5FHControlCommand& other) const
  {
    return
      (position == other.position);
  }

};


//! overload stream operator to easily serialize data
//! slightly uneccessary at this point but put in anayway to make sure it is used :)
inline icl_comm::ArrayBuilder& operator << (icl_comm::ArrayBuilder& ab, const S5FHControlCommand& data)
{
  ab << data.position;
  return ab;
}


//! overload stream operator to easily deserialize data
inline icl_comm::ArrayBuilder& operator >> (icl_comm::ArrayBuilder& ab, S5FHControlCommand& data)
{
  ab >> data.position;
  return ab;
}

//! Output Stream operator
inline std::ostream& operator << (std::ostream& o, const S5FHControlCommand& cc)
{
  o << "Pos: " << cc.position << std::endl;
  return o;
}


}

#endif // S5FHCONTROLCOMMAND_H
