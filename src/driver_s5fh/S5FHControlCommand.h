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
  //! \param _position target position given in encoder ticks defaults to 0 if none is given
  //!
  S5FHControlCommand(const int32_t& _position = 0):
    position(_position)
  {}

  //! Compares two S5FHControlCommand objects.
  bool operator == (const S5FHControlCommand& other) const
  {
    return
      (position == other.position);
  }

};

/*!
 * \brief Structure for transmitting all controllcommands at once
 */
struct S5FHControlCommandAllChannels
{
  //! Multiple controllcommands that shall be send at once
  std::vector<S5FHControlCommand> commands;

  // TODO: Make a constructor with a variable number of arguments and correct mapping that is not completely ugly
  //!
  //! \brief Constructs a controllcommand adressing all channels at once All
  //! \param _position0 Target position for the Thumb_Flexion
  //! \param _position1 Target position for the Thumb_Opposition
  //! \param _position2 Target position for the Index_Finger_Distal
  //! \param _position3 Target position for the Index_Finger_Proximal
  //! \param _position4 Target position for the Middle_Finger_Distal
  //! \param _position5 Target position for the Middle_Finger_Proximal
  //! \param _position6 Target position for the Ring_Finger
  //! \param _position7 Target position for the Pinky
  //! \param _position8 Target position for the Finger_Spread
  //!
  S5FHControlCommandAllChannels(const int32_t& _position0,const int32_t& _position1,const int32_t& _position2,
                                const int32_t& _position3,const int32_t& _position4,const int32_t& _position5,
                                const int32_t& _position6,const int32_t& _position7,const int32_t& _position8)
  {
    commands.push_back(S5FHControlCommand(_position0));
    commands.push_back(S5FHControlCommand(_position1));
    commands.push_back(S5FHControlCommand(_position2));
    commands.push_back(S5FHControlCommand(_position3));
    commands.push_back(S5FHControlCommand(_position4));
    commands.push_back(S5FHControlCommand(_position5));
    commands.push_back(S5FHControlCommand(_position6));
    commands.push_back(S5FHControlCommand(_position7));
    commands.push_back(S5FHControlCommand(_position8));
  }

  /*!
   * \brief Construct a control command for all channels from a vector. Only the first 9 Values are used
   * \param positions Vector of position values. Only the first 9 values are evaluated
   */
  S5FHControlCommandAllChannels(const std::vector<int32_t>& positions)
  {
    commands.insert(commands.begin(),positions.begin(),positions.begin()+9);
  }

  /*!
   * \brief Constructs an empty S5FHControlCommandAllChannels structure pre filled with 9 empty S5FHControlCommands. Mainly usefull for deserialisation
   */
  S5FHControlCommandAllChannels():
    commands(9,S5FHControlCommand())
  { }

  //! Compares two S5FHControlCommand objects.
  bool operator == (const S5FHControlCommandAllChannels& other) const
  {
    return
      (commands == other.commands);
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

//! overload stream operator to easily serialize data
inline icl_comm::ArrayBuilder& operator << (icl_comm::ArrayBuilder& ab, const S5FHControlCommandAllChannels& data)
{
  // We could also just give the whole vector in ...
  for (std::vector<S5FHControlCommand>::const_iterator it = data.commands.begin() ; it != data.commands.end(); ++it)
  {
    ab << *it;
  }
  return ab;
}


//! overload stream operator to easily deserialize data
inline icl_comm::ArrayBuilder& operator >> (icl_comm::ArrayBuilder& ab, S5FHControlCommandAllChannels& data)
{
  for (std::vector<S5FHControlCommand>::iterator it = data.commands.begin() ; it != data.commands.end(); ++it)
  {
    ab >> *it;
  }
  return ab;
}


//! Output Stream operator
inline std::ostream& operator << (std::ostream& o, const S5FHControlCommandAllChannels& cc)
{
  o << "Commands: " ;
  unsigned int i = 0;
  for (std::vector<S5FHControlCommand>::const_iterator it = cc.commands.begin() ; it != cc.commands.end(); ++it)
  {
    o << "Chan " << i << " : "<< *it;
    ++i;
  }
  o << std::endl;
  return o;
}






}

#endif // S5FHCONTROLCOMMAND_H
