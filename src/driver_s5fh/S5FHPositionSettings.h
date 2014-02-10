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
#ifndef S5FHPOSITIONSETTINGS_H
#define S5FHPOSITIONSETTINGS_H

#include <icl_core/BaseTypes.h>
#include <icl_comm/ByteOrderConversion.h>

namespace driver_s5fh {

/*!
 * \brief The S5FHPositionSettings save the position controller paramters for a single motor
 */
struct S5FHPositionSettings
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
};

//! overload stream operator to easily serialize data
inline icl_comm::ArrayBuilder& operator << (icl_comm::ArrayBuilder& ab, const S5FHPositionSettings& data)
{
  ab << data.wmn
     << data.wmx
     << data.dwmx
     << data.ky
     << data.dt
     << data.imn
     << data.imx
     << data.kp
     << data.ki
     << data.kd;
  return ab;
}




}

#endif // S5FHPOSITIONSETTINGS_H
