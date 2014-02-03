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

namespace driver_s5fh {

/*!
 * \brief The S5FHPositionSettings save the position controller paramters for a single motor
 */
struct S5FHPositionSettings
{
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


//ArrayBuilder& operator << (ArrayBuilder& ab,const S5FHPositionSettings& data);

}

#endif // S5FHPOSITIONSETTINGS_H
