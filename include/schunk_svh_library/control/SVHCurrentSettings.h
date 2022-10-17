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
 * This file contains the CurrentSettings data structure that is used to
 * send and receive the settings for the current controller in the position/current cascade
 */
//----------------------------------------------------------------------
#ifndef SVHCURRENTSETTINGS_H
#define SVHCURRENTSETTINGS_H

#include <schunk_svh_library/serial/ByteOrderConversion.h>

namespace driver_svh {

/*!
 * \brief The SVHCurrentSettings save the current controller paramters for a single motor
 */
struct SVHCurrentSettings
{
  //! Reference signal minimum value
  float wmn;
  //! Reference signal maximum value
  float wmx;
  //! measurement scaling
  float ky;
  //! time base of controller
  float dt;
  //! Integral windup minimum value
  float imn;
  //! Integral windup maximum value
  float imx;
  //! Proportional gain
  float kp;
  //! Integral gain
  float ki;
  //! Output limiter min
  float umn;
  //! Output limiter max
  float umx;

  //! Compares two SVHCurrentSettings objects.
  bool operator==(const SVHCurrentSettings& other) const
  {
    return (wmn == other.wmn && wmx == other.wmx && ky == other.ky && dt == other.dt &&
            imn == other.imn && imx == other.imx && kp == other.kp && ki == other.ki &&
            umn == other.umn && umx == other.umx);
  }

  //!
  //! \brief SVHCurrentSettings Construct current settings from a plain vector if the Vector is
  //! smaler than 10 values will be filled with 0.0 \param cur_settings Vector of floats in the
  //! order: wmn, wmx, ky, dt, imn, imx, kp, ki , umn, umx
  //!
  SVHCurrentSettings(const std::vector<float>& cur_settings)
  {
    size_t size = cur_settings.size();

    // is there a nicer way to do this? Please tell me if there is :)
    wmn = (size > 0) ? cur_settings[0] : 0.0f;
    wmx = (size > 1) ? cur_settings[1] : 0.0f;
    ky  = (size > 2) ? cur_settings[2] : 0.0f;
    dt  = (size > 3) ? cur_settings[3] : 0.0f;
    imn = (size > 4) ? cur_settings[4] : 0.0f;
    imx = (size > 5) ? cur_settings[5] : 0.0f;
    kp  = (size > 6) ? cur_settings[6] : 0.0f;
    ki  = (size > 7) ? cur_settings[7] : 0.0f;
    umn = (size > 8) ? cur_settings[8] : 0.0f;
    umx = (size > 9) ? cur_settings[9] : 0.0f;
  }

  //!
  //! \brief SVHCurrentSettings Constructs a new current settings object to configure the current
  //! controller of a finger \param wmn Reference signal minimum value \param wmx Reference signal
  //! maximum value \param ky measurement scaling \param dt time base of controller \param imn
  //! Integral windup minimum value \param imx Integral windup maximum value \param kp  Proportional
  //! gain \param ki  Integral gain \param umn Output limiter min \param umx Output limiter max
  //!
  SVHCurrentSettings(const float& wmn,
                     const float& wmx,
                     const float& ky,
                     const float& dt,
                     const float& imn,
                     const float& imx,
                     const float& kp,
                     const float& ki,
                     const float& umn,
                     const float& umx)
    : wmn(wmn)
    , wmx(wmx)
    , ky(ky)
    , dt(dt)
    , imn(imn)
    , imx(imx)
    , kp(kp)
    , ki(ki)
    , umn(umn)
    , umx(umx)
  {
  }

  //!
  //! \brief SVHCurrentSettings Default constructor, initalizes everything to zero
  //!
  SVHCurrentSettings()
    : wmn(0.0)
    , wmx(0.0)
    , ky(0.0)
    , dt(0.0)
    , imn(0.0)
    , imx(0.0)
    , kp(0.0)
    , ki(0.0)
    , umn(0.0)
    , umx(0.0)
  {
  }
};

//! overload stream operator to easily serialize current settings data
inline driver_svh::ArrayBuilder& operator<<(driver_svh::ArrayBuilder& ab,
                                            const SVHCurrentSettings& data)
{
  ab << data.wmn << data.wmx << data.ky << data.dt << data.imn << data.imx << data.kp << data.ki
     << data.umn << data.umx;
  return ab;
}

//! overload stream operator to easily serialize current settings data
inline driver_svh::ArrayBuilder& operator>>(driver_svh::ArrayBuilder& ab, SVHCurrentSettings& data)
{
  ab >> data.wmn >> data.wmx >> data.ky >> data.dt >> data.imn >> data.imx >> data.kp >> data.ki >>
    data.umn >> data.umx;
  return ab;

  return ab;
}

//! Output stream operator for easy output of current settings
inline std::ostream& operator<<(std::ostream& o, const SVHCurrentSettings& cs)
{
  o << "wmn " << cs.wmn << " "
    << "wmx " << cs.wmx << " "
    << "ky " << cs.ky << " "
    << "dt " << cs.dt << " "
    << "imn " << cs.imn << " "
    << "imx " << cs.imx << " "
    << "kp " << cs.kp << " "
    << "ki " << cs.ki << " "
    << "umn " << cs.umn << " "
    << "umx " << cs.umx << " " << std::endl;
  return o;
}


} // namespace driver_svh
#endif // SVHCURRENTSETTINGS_H
