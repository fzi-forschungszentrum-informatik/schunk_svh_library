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
#ifndef S5FHCONTROLLERFEEDBACK_H
#define S5FHCONTROLLERFEEDBACK_H

#include <icl_core/BaseTypes.h>

namespace driver_s5fh {

/*!
 * \brief The S5FHCurrentSettings save the current controller paramters for a single motor
 */
struct S5FHControllerFeedback
{
  //! Returned position value of the motor [Ticks]
  int32_t position;
  //! Returned current value of the motor [mA?]
  int16_t current;

  //! Compares two S5FHControllerFeedback objects.
  bool operator == (const S5FHControllerFeedback& other) const
  {
    return
      (position == other.position
       && current == other.current)
        ;
  }
};

//! overload stream operator to easily serialize data
inline icl_comm::ArrayBuilder& operator << (icl_comm::ArrayBuilder& ab, S5FHControllerFeedback& data)
{
  ab << data.position
     << data.current;
  return ab;
}


//! overload stream operator to easily deserialize data
inline icl_comm::ArrayBuilder& operator >> (icl_comm::ArrayBuilder& ab, S5FHControllerFeedback& data)
{
  ab >> data.current
     >> data.position;
  return ab;
}

//! Output Stream operator
inline std::ostream& operator << (std::ostream& o, const S5FHControllerFeedback& cf)
{
  o << "Pos: " << cf.position << " Cur: " << cf.current << std::endl;
  return o;
}


}


#endif // S5FHCONTROLLERFEEDBACK_H
