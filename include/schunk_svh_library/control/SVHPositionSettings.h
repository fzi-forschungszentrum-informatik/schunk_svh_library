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
 * This file contains the SVHPositionSettings data structure that is used to
 * receive and send the current parameters of the position controller within the hand.
 */
//----------------------------------------------------------------------
#ifndef SVHPOSITIONSETTINGS_H
#define SVHPOSITIONSETTINGS_H

#include <schunk_svh_library/serial/ByteOrderConversion.h>

namespace driver_svh {

/*!
 * \brief The SVHPositionSettings save the position controller paramters for a single motor
 */
struct SVHPositionSettings
{
public:
  //! Reference signal minimum value
  float wmn;
  //! Reference signal maximum value
  float wmx;
  //! Reference signal delta maximum threshold
  float dwmx;
  //! Measurement scaling
  float ky;
  //! Time base of controller
  float dt;
  //! Integral windup minimum value
  float imn;
  //! Integral windup maximum value
  float imx;
  //! Proportional gain
  float kp;
  //! Integral gain
  float ki;
  //! Differential gain
  float kd;

  //! Compares two SVHPositionsetting objects.
  bool operator==(const SVHPositionSettings& other) const
  {
    return (wmn == other.wmn && wmx == other.wmx && dwmx == other.dwmx && ky == other.ky &&
            dt == other.dt && imn == other.imn && imx == other.imx && kp == other.kp &&
            ki == other.ki && kd == other.kd);
  }

  //!
  //! \brief SVHPositionSettings Construct positions settings from a plain vector if the Vector is
  //! smaler than 10 values will be filled with 0.0 \param pos_settings Vector of floats in the
  //! order: wmn, wmx, dwmx, ky, dt, imn, imx, kp, ki , kd
  //!
  SVHPositionSettings(const std::vector<float>& pos_settings)
  {
    size_t size = pos_settings.size();

    wmn  = (size > 0) ? pos_settings[0] : 0.0f;
    wmx  = (size > 1) ? pos_settings[1] : 0.0f;
    dwmx = (size > 2) ? pos_settings[2] : 0.0f;
    ky   = (size > 3) ? pos_settings[3] : 0.0f;
    dt   = (size > 4) ? pos_settings[4] : 0.0f;
    imn  = (size > 5) ? pos_settings[5] : 0.0f;
    imx  = (size > 6) ? pos_settings[6] : 0.0f;
    kp   = (size > 7) ? pos_settings[7] : 0.0f;
    ki   = (size > 8) ? pos_settings[8] : 0.0f;
    kd   = (size > 9) ? pos_settings[9] : 0.0f;
  }


  //!
  //! \brief SVHPositionSettings Constructs a new positition settings object to configure the
  //! position controller of a finger \param wmn Reference signal minimum value \param wmx Reference
  //! signal maximum value \param dwmx Reference signal delta maximum threshold \param ky
  //! Measurement scaling \param dt Time base of controller \param imn Integral windup minimum value
  //! \param imx Integral windup maximum value
  //! \param kp Proportional gain
  //! \param ki Integral gain
  //! \param kd Differential gain
  //!
  SVHPositionSettings(const float& wmn,
                      const float& wmx,
                      const float& dwmx,
                      const float& ky,
                      const float& dt,
                      const float& imn,
                      const float& imx,
                      const float& kp,
                      const float& ki,
                      const float& kd)
    : wmn(wmn)
    , wmx(wmx)
    , dwmx(dwmx)
    , ky(ky)
    , dt(dt)
    , imn(imn)
    , imx(imx)
    , kp(kp)
    , ki(ki)
    , kd(kd)
  {
  }

  //!
  //! \brief SVHPositionSettings Defaault constructor. Intitializes everything to zero
  //!
  SVHPositionSettings()
    : wmn(0.0)
    , wmx(0.0)
    , dwmx(0.0)
    , ky(0.0)
    , dt(0.0)
    , imn(0.0)
    , imx(0.0)
    , kp(0.0)
    , ki(0.0)
    , kd(0.0)
  {
  }
};

//! overload stream operator to easily serialize position settings data
inline driver_svh::ArrayBuilder& operator<<(driver_svh::ArrayBuilder& ab,
                                            const SVHPositionSettings& data)
{
  ab << data.wmn << data.wmx << data.dwmx << data.ky << data.dt << data.imn << data.imx << data.kp
     << data.ki << data.kd;
  return ab;
}

//! overload stream operator to easily deserialize position settings data
inline driver_svh::ArrayBuilder& operator>>(driver_svh::ArrayBuilder& ab, SVHPositionSettings& data)
{
  ab >> data.wmn >> data.wmx >> data.dwmx >> data.ky >> data.dt >> data.imn >> data.imx >>
    data.kp >> data.ki >> data.kd;
  return ab;
}

//! Output stream operator to easily print position settings
inline std::ostream& operator<<(std::ostream& o, const SVHPositionSettings& ps)
{
  o << "wmn " << ps.wmn << " "
    << "wmx " << ps.wmx << " "
    << "dwmx " << ps.dwmx << " "
    << "ky " << ps.ky << " "
    << "dt " << ps.dt << " "
    << "imn " << ps.imn << " "
    << "imx " << ps.imx << " "
    << "kp " << ps.kp << " "
    << "ki " << ps.ki << " "
    << "kd " << ps.kd << " " << std::endl;
  return o;
}


} // namespace driver_svh

#endif // SVHPOSITIONSETTINGS_H
