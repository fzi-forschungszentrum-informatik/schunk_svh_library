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
#ifndef S5FHCURRENTSETTINGS_H
#define S5FHCURRENTSETTINGS_H

#include <icl_core/BaseTypes.h>
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
};

//! overload stream operator to easily serialize data
template <typename TArray>
icl_comm::ArrayBuilder<TArray>& operator << (icl_comm::ArrayBuilder<TArray>& ab, const S5FHCurrentSettings& data)
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

}



#endif // S5FHCURRENTSETTINGS_H
