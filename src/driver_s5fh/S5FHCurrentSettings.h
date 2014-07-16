// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

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
#ifndef S5FHCURRENTSETTINGS_H
#define S5FHCURRENTSETTINGS_H

#include <icl_comm/ByteOrderConversion.h>

namespace driver_s5fh {

/*!
 * \brief The S5FHCurrentSettings save the current controller paramters for a single motor
 */
struct S5FHCurrentSettings
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
  //! proportional gain
  float kp;
  //! Integral gain
  float ki;
  //! Output limiter min
  float umn;
  //! Output limiter max
  float umx;

  //! Compares two S5FHCurrentSettings objects.
  bool operator == (const S5FHCurrentSettings& other) const
  {
    return
      (wmn == other.wmn
       && wmx == other.wmx
       && ky == other.ky
       && dt == other.dt
       && imn == other.imn
       && imx == other.imx
       && kp == other.kp
       && ki == other.ki
       && umn == other.umn
       && umx == other.umx
       );
  }

};

//! overload stream operator to easily serialize current settings data
inline icl_comm::ArrayBuilder& operator << (icl_comm::ArrayBuilder& ab, const S5FHCurrentSettings& data)
{
  ab << data.wmn
     << data.wmx
     << data.ky
     << data.dt
     << data.imn
     << data.imx
     << data.kp
     << data.ki
     << data.umn
     << data.umx;
  return ab;
}

//! overload stream operator to easily serialize current settings data
inline icl_comm::ArrayBuilder& operator >> (icl_comm::ArrayBuilder& ab, S5FHCurrentSettings& data)
{
  ab >> data.wmn
     >> data.wmx
     >> data.ky
     >> data.dt
     >> data.imn
     >> data.imx
     >> data.kp
     >> data.ki
     >> data.umn
     >> data.umx;
  return ab;

  return ab;
}

//! Output stream operator for easy output of current settings
inline std::ostream& operator << (std::ostream& o, const S5FHCurrentSettings& cs)
{
  o << "wmn "<< cs.wmn << " "
    << "wmx "<< cs.wmx << " "
    << "ky " << cs.ky  << " "
    << "dt " << cs.dt  << " "
    << "imn "<< cs.imn << " "
    << "imx "<< cs.imx << " "
    << "kp " << cs.kp  << " "
    << "ki " << cs.ki  << " "
    << "umn "<< cs.umn << " "
    << "umx "<< cs.umx << " "
    << std::endl;
  return o;
}


}
#endif // S5FHCURRENTSETTINGS_H
