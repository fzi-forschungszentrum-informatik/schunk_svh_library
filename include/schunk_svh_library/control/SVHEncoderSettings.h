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
 * \date    2014-02-11
 * \date    2014-07-16
 *
 * This file contains the EncoderSettings data structure that is used to
 * send and receive the settings for the encoders. Basically these are multipliers
 * and could be use to pre scale the encoders to meaningfull values (i.e. rad)
 * As this is done by this driver, sending them should never be necessary
 */
//----------------------------------------------------------------------
#ifndef SVHENCODERSETTINGS_H
#define SVHENCODERSETTINGS_H

#include <schunk_svh_library/serial/ByteOrderConversion.h>

namespace driver_svh {

/*!
 * \brief The SVHEncoderSettings hold the settings for the encoder scaling of each channel
 */
struct SVHEncoderSettings
{
  //! encoderSettings consist of multipliers for each encoder
  std::vector<uint32_t> scalings;

  // TODO Provide a constructor that allows for seperate encoder settings in the hardware
  /*!
   * \brief SVHEncoderSettings Default CTOR will assign 9x1 to the scalings if no argument is given
   * \param _scaling scaling to use for the encoders (everyone is scaled the same)
   */
  SVHEncoderSettings(uint32_t scaling = 1)
    : scalings(9, scaling)
  {
  }

  //! Compares two SVHEncoderSettings objects.
  bool operator==(const SVHEncoderSettings& other) const { return (scalings == other.scalings); }
};


//! overload stream operator to easily serialize encoder scaling data
inline driver_svh::ArrayBuilder& operator<<(driver_svh::ArrayBuilder& ab,
                                            const SVHEncoderSettings& data)
{
  // Trivial as the vector slicing is already done by the arraybuilder
  ab << data.scalings;
  return ab;
}

//! overload stream operator to easily serialize encoder scaling data
inline driver_svh::ArrayBuilder& operator>>(driver_svh::ArrayBuilder& ab, SVHEncoderSettings& data)
{
  ab >> data.scalings;
  return ab;
}


//! Output Stream operator for easy output of the encoder scalings
inline std::ostream& operator<<(std::ostream& o, const SVHEncoderSettings& es)
{
  o << "Scalings: ";
  for (size_t i = 0; i < es.scalings.size(); i++)
  {
    o << (int)i << ":" << es.scalings[i] << " ";
  }

  o << std::endl;
  return o;
}

} // namespace driver_svh
#endif // SVHENCODERSETTINGS_H
