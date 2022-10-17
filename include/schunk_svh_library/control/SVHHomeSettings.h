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
 * \author  Georg Heppner <heppner@fzi.de>
 * \date    2014-9-18
 *
 * This file contains the HomeSettings data structure that is used to
 * specify the min and max positions as well as the homing offset used during reset and the
 * movement direction of the fingers
 */
//----------------------------------------------------------------------

#ifndef SVHHOMESETTINGS_H
#define SVHHOMESETTINGS_H

#include <ostream>
#include <vector>

namespace driver_svh {

//! data sctructure for home positions
struct SVHHomeSettings
{
  //! Movement direction of the finger +1 or -1 home in positive or negative direction
  int direction;
  //! Minimum reachable tick limit, given as offset from the hard stop (soft limit)
  float minimum_offset;
  //! Maximum reachable tick limit, given as an offset from the hard stop (soft limit)
  float maximum_offset;
  //! Idle position to move the fingers to after initialization @note This position should be within
  //! the min and max offset positions.
  float idle_position;
  //! The total angle in [rad] that is used by the finger i.e. the anlge between one (soft) stop and
  //! the other This value will be used for calculation of the ticks
  //! @note Currently we use the HARD STOP to HARD STOP angle but map that to a range of ticks from
  //! SOFT STOP to SOFT STOP this might be the wrong approach
  //! @note as we therefore would get an angle of zero event hough the finger might be standing at
  //! 5k ticks. However by using this method we can asure the Range of the
  //! @note input to be of constant size. This night change to better reflect the real rad value
  float range_rad;
  //! The resetCurrentFactor indicates how much of the maximum allowed current (of the controller)
  //! must be present in order to decide that a hard stop was reached. @note Values betweeen 0.0
  //! and 1.0 are allowed
  float reset_current_factor;

  //!
  //! \brief SVHHomeSettings Default constructor initializing empty homeSettings. This is not
  //! usefull and should be filled immediately after
  //!
  SVHHomeSettings()
    : direction(+1)
    , minimum_offset(0.0)
    , maximum_offset(0.0)
    , idle_position(0.0)
    , range_rad(0.0)
    , reset_current_factor(0.5)
  {
  }

  //!
  //! \brief SVHHomeSettingsn Constructor for the home settings, providing each value individually
  //! \param direction_ Reset direction +1 oder -1
  //! \param minimumOffset_ soft stop minimum value
  //! \param maximumOffset_ soft stop maximum value
  //! \param idlePosition_ position to go to after reset
  //! \param rangeRad_ range of moevement in rad
  //!
  SVHHomeSettings(const int& direction,
                  const float& minimum_offset,
                  const float& maximum_offset,
                  const float& idle_position,
                  const float& range_rad,
                  const float& reset_current_factor)
    : minimum_offset(minimum_offset)
    , maximum_offset(maximum_offset)
    , idle_position(idle_position)
    , range_rad(range_rad)
    , reset_current_factor(reset_current_factor)
  {
    // Limit the direction to just contain a factor rather than any multipliers
    this->direction = direction < 0 ? -1 : +1;
    // Limit the resetCurrentFactor to allowed bounds
    this->reset_current_factor = std::max(std::min(reset_current_factor, 1.0f), 0.0f);
  }

  //!
  //! \brief SVHHomeSettings Convenience constructor to easily construct home settings out of a
  //! singe vector. Values are evaluated in order and if the vector is to short, values will be
  //! filles with zeros \param home_settings vector of floats containing the values for the home
  //! settings. If the vector is to short the remaining values will be filled with zeros. If it is
  //! to long only as much elements as there are values in HomeSettings will be evaluated
  //!
  SVHHomeSettings(const std::vector<float>& home_settings)
  {
    // maybe not the most beautifull way but it works.
    size_t size          = home_settings.size();
    float temp_direction = 0.0;

    temp_direction = (size > 0) ? home_settings[0] : 0.0f;
    direction      = temp_direction < 0.0 ? -1 : +1;

    minimum_offset       = (size > 1) ? home_settings[1] : 0.0f;
    maximum_offset       = (size > 2) ? home_settings[2] : 0.0f;
    idle_position        = (size > 3) ? home_settings[3] : 0.0f;
    range_rad            = (size > 4) ? home_settings[4] : 0.0f;
    reset_current_factor = (size > 5) ? home_settings[5] : 0.5f;

    // Limit the resetCurrentFactor to allowed bounds
    reset_current_factor = std::max(std::min(reset_current_factor, 1.0f), 0.0f);
  }
};


//! Output stream operator to easily print position settings
inline std::ostream& operator<<(std::ostream& o, const SVHHomeSettings& hs)
{
  o << "Direction " << hs.direction << " "
    << "Min offset " << hs.minimum_offset << " "
    << "Max offset " << hs.maximum_offset << " "
    << "idle pos " << hs.idle_position << " "
    << "Range Rad " << hs.range_rad << " "
    << "Reset Curr Factor " << hs.reset_current_factor << " " << std::endl;
  return o;
}


} // namespace driver_svh


#endif // SVHHOMESETTINGS_H
