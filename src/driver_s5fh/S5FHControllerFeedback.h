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

  //!
  //! \brief Standard constructor
  //! \param _position Intitial position value, defaults to 0
  //! \param _current Initital current value, defaults to 0
  //!
  S5FHControllerFeedback(int32_t _position = 0,int16_t _current = 0):
    position(_position),
    current(_current)
  {

  }

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
  // Dirty? Hack.. if the Arraybuilder is longer then the things we expect we just overwrite that..
  // This will be a problem if for example you would want to write ab >> data_struct >>data_struct
  // We will think of something better later.. for now we just expect to be a packet the size we want it to be
  if (ab.pos > 6)
  {
    ab.pos = 6;
  }
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
