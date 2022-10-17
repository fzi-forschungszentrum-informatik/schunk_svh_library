////////////////////////////////////////////////////////////////////////////////
//
// © Copyright 2022 SCHUNK Mobile Greifsysteme GmbH, Lauffen/Neckar Germany
// © Copyright 2022 FZI Forschungszentrum Informatik, Karlsruhe, Germany
//
// This file is part of the Schunk SVH Library.
//
// The Schunk SVH Library is free software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
//
// The Schunk SVH Library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
// Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// the Schunk SVH Library. If not, see <https://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Georg Heppner
 * \date    2014-02-03
 * \date    2014-07-16
 *
 * This file contains the ControlCommand Data Structure that is used to
 * transmit target positions an neccessary helper functions
 */
//----------------------------------------------------------------------
#ifndef SVHCONTROLCOMMAND_H
#define SVHCONTROLCOMMAND_H

#include <schunk_svh_library/serial/ByteOrderConversion.h>

namespace driver_svh {

/*!
 * \class SVHControlCommand
 * \brief ControlCommands are given as a single target position for the position controller (given
 * in ticks)
 */
struct SVHControlCommand
{
  //! Returned position value of the motor [Ticks]
  int32_t position;

  /**
   * \brief Constructs a new control command to comandeer the position of the fingers
   * \param _position target position given in encoder ticks defaults to 0 if none is given
   **/
  SVHControlCommand(const int32_t& position = 0)
    : position(position)
  {
  }

  //! Compares two SVHControlCommand objects.
  bool operator==(const SVHControlCommand& other) const { return (position == other.position); }
};

/*!
 * \class SVHControlCommandAllChannels
 * \brief Structure for transmitting all controllcommands at once
 */
struct SVHControlCommandAllChannels
{
  //! Multiple controllcommands that shall be send at once
  std::vector<SVHControlCommand> commands;

  /**
   * \brief Constructs a controllcommand adressing all channels at once All
   * \param _position0 Target position for the Thumb_Flexion
   * \param _position1 Target position for the Thumb_Opposition
   * \param _position2 Target position for the Index_Finger_Distal
   * \param _position3 Target position for the Index_Finger_Proximal
   * \param _position4 Target position for the Middle_Finger_Distal
   * \param _position5 Target position for the Middle_Finger_Proximal
   * \param _position6 Target position for the Ring_Finger
   * \param _position7 Target position for the Pinky
   * \param _position8 Target position for the Finger_Spread
   **/
  SVHControlCommandAllChannels(const int32_t& position0,
                               const int32_t& position1,
                               const int32_t& position2,
                               const int32_t& position3,
                               const int32_t& position4,
                               const int32_t& position5,
                               const int32_t& position6,
                               const int32_t& position7,
                               const int32_t& position8)
  {
    commands.push_back(SVHControlCommand(position0));
    commands.push_back(SVHControlCommand(position1));
    commands.push_back(SVHControlCommand(position2));
    commands.push_back(SVHControlCommand(position3));
    commands.push_back(SVHControlCommand(position4));
    commands.push_back(SVHControlCommand(position5));
    commands.push_back(SVHControlCommand(position6));
    commands.push_back(SVHControlCommand(position7));
    commands.push_back(SVHControlCommand(position8));
  }

  /*!
   * \brief Construct a control command for all channels from a vector. Only the first 9 Values are
   * used \param positions vector of position values. Only the first 9 values are evaluated
   */
  SVHControlCommandAllChannels(const std::vector<int32_t>& positions)
  {
    commands.insert(commands.begin(), positions.begin(), positions.begin() + 9);
  }

  /*!
   * \brief Constructs an empty SVHControlCommandAllChannels structure pre filled with 9 empty
   * SVHControlCommands. Mainly usefull for deserialisation
   */
  SVHControlCommandAllChannels()
    : commands(9, SVHControlCommand())
  {
  }

  //! Compares two SVHControlCommand objects.
  bool operator==(const SVHControlCommandAllChannels& other) const
  {
    return (commands == other.commands);
  }
};


//! overload stream operator to easily serialize control commands for one channel
//! slightly uneccessary at this point but put in anayway for the time it`s needed
inline driver_svh::ArrayBuilder& operator<<(driver_svh::ArrayBuilder& ab,
                                            const SVHControlCommand& data)
{
  ab << data.position;
  return ab;
}


//! overload stream operator to easily deserialize control commands for one channel
inline driver_svh::ArrayBuilder& operator>>(driver_svh::ArrayBuilder& ab, SVHControlCommand& data)
{
  ab >> data.position;
  return ab;
}

//! Output Stream operator for fast debugging
inline std::ostream& operator<<(std::ostream& o, const SVHControlCommand& cc)
{
  o << "Pos: " << cc.position << std::endl;
  return o;
}

//! overload stream operator to easily serialize control commands for all channels
inline driver_svh::ArrayBuilder& operator<<(driver_svh::ArrayBuilder& ab,
                                            const SVHControlCommandAllChannels& data)
{
  // We could also just give the whole vector in ...
  for (std::vector<SVHControlCommand>::const_iterator it = data.commands.begin();
       it != data.commands.end();
       ++it)
  {
    ab << *it;
  }
  return ab;
}


//! overload stream operator to easily deserialize control commands for all channels
inline driver_svh::ArrayBuilder& operator>>(driver_svh::ArrayBuilder& ab,
                                            SVHControlCommandAllChannels& data)
{
  for (std::vector<SVHControlCommand>::iterator it = data.commands.begin();
       it != data.commands.end();
       ++it)
  {
    ab >> *it;
  }
  return ab;
}


//! Output Stream operator of all channels control command
inline std::ostream& operator<<(std::ostream& o, const SVHControlCommandAllChannels& cc)
{
  o << "Commands: ";
  unsigned int i = 0;
  for (std::vector<SVHControlCommand>::const_iterator it = cc.commands.begin();
       it != cc.commands.end();
       ++it)
  {
    o << "Chan " << i << " : " << *it;
    ++i;
  }
  o << std::endl;
  return o;
}
} // namespace driver_svh

#endif // SVHCONTROLCOMMAND_H
