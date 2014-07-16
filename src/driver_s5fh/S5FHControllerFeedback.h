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
 * This file contains the ControllerFeedback data structure that is used to
 * receive feedback of the hardware indicating the current position and
 * current values for each channel (finger)
 */
//----------------------------------------------------------------------
#ifndef S5FHCONTROLLERFEEDBACK_H
#define S5FHCONTROLLERFEEDBACK_H

#include <icl_comm/ByteOrderConversion.h>

namespace driver_s5fh {

/*!
 * \brief The S5FHControllerFeedback saves the feedback of a single motor
 */
struct S5FHControllerFeedback
{
  //! Returned position value of the motor [Ticks]
  int32_t position;
  //! Returned current value of the motor [mA]
  int16_t current;

  /*!
   * \brief standard constructor
   * \param _position Intitial position value, defaults to 0
   * \param _current Initital current value, defaults to 0
  **/
  S5FHControllerFeedback(const int32_t& _position = 0,const int16_t& _current = 0):
    position(_position),
    current(_current)
  {}

  //! Compares two S5FHControllerFeedback objects.
  bool operator == (const S5FHControllerFeedback& other) const
  {
    return (position == other.position && current == other.current);
  }
};

/*!
 * \brief The S5FHControllerFeedbackAllChannes saves the feedback of a all motors
 */
struct S5FHControllerFeedbackAllChannels
{
  //! Vector holding multiple channels
  std::vector<S5FHControllerFeedback> feedbacks;

  /*!
   * \brief Constructs a S5FHControllerFeedbackAllChannels data structure from explicit ffedback elements
   * \param _feedback0 Feedback for the Thumb_Flexion
   * \param _feedback1 Feedback for the Thumb_Opposition
   * \param _feedback2 Feedback for the Index_Finger_Distal
   * \param _feedback3 Feedback for the Index_Finger_Proximal
   * \param _feedback4 Feedback for the Middle_Finger_Distal
   * \param _feedback5 Feedback for the Middle_Finger_Proximal
   * \param _feedback6 Feedback for the Ring_Finger
   * \param _feedback7 Feedback for the Pinky
   * \param _feedback8 Feedback for the Finger_Spread
   */
  S5FHControllerFeedbackAllChannels(const S5FHControllerFeedback& _feedback0,const S5FHControllerFeedback& _feedback1,
                                    const S5FHControllerFeedback& _feedback2,const S5FHControllerFeedback& _feedback3,
                                    const S5FHControllerFeedback& _feedback4,const S5FHControllerFeedback& _feedback5,
                                    const S5FHControllerFeedback& _feedback6,const S5FHControllerFeedback& _feedback7,
                                    const S5FHControllerFeedback& _feedback8)
  {
    feedbacks.push_back(_feedback0);
    feedbacks.push_back(_feedback1);
    feedbacks.push_back(_feedback2);
    feedbacks.push_back(_feedback3);
    feedbacks.push_back(_feedback4);
    feedbacks.push_back(_feedback5);
    feedbacks.push_back(_feedback6);
    feedbacks.push_back(_feedback7);
    feedbacks.push_back(_feedback8);
  }

  /*!
   * \brief Creates a S5FHControllerFeedbackAllChannels structure from a vector
   * \param _feedbacks Vector filled with S5FHControllerFeedback elements.
   * \note Alhough it is possible to create a feedback vector with more than 9 elements, that would be rather pointles as we only have 9 chanels less than 9 channels will result in only partial feedback
   */
  S5FHControllerFeedbackAllChannels(std::vector<S5FHControllerFeedback> _feedbacks)
  {
    feedbacks.insert(feedbacks.begin(),_feedbacks.begin(),_feedbacks.end());
  }

  /*!
   * \brief Constructs an empty S5FHControllerFeedbackAllChannels objects, prefilled with 9 default channel feedbacks, mainly usefull for deserialization
   */
  S5FHControllerFeedbackAllChannels():
    feedbacks(9,S5FHControllerFeedback())
  { }


  //! Compares two S5FHControllerFeedbackAllChannels objects.
  bool operator == (const S5FHControllerFeedbackAllChannels& other) const
  {
    return (feedbacks == other.feedbacks);
  }
};

//! Overload stream operator to easily serialize feedback data
inline icl_comm::ArrayBuilder& operator << (icl_comm::ArrayBuilder& ab,const S5FHControllerFeedback& data)
{
  ab << data.position
     << data.current;
  return ab;
}


//! Overload stream operator to easily deserialize feedback data
inline icl_comm::ArrayBuilder& operator >> (icl_comm::ArrayBuilder& ab, S5FHControllerFeedback& data)
{
  ab >> data.position
     >> data.current;
  return ab;
}

//! Output stream operator for easy output of feedback data
inline std::ostream& operator << (std::ostream& o, const S5FHControllerFeedback& cf)
{
  o << "Pos: " << cf.position << " Cur: " << cf.current << std::endl;
  return o;
}


//! Overload stream operator to easily serialize all channel feedback data
inline icl_comm::ArrayBuilder& operator << (icl_comm::ArrayBuilder& ab,S5FHControllerFeedbackAllChannels& data)
{
  // The Data is transmitted not channel by channel but rather position first, Currents afterwards for all channels
  for (std::vector<S5FHControllerFeedback>::iterator it = data.feedbacks.begin() ; it != data.feedbacks.end(); ++it)
  {
    ab << it->position;
  }

  for (std::vector<S5FHControllerFeedback>::iterator it = data.feedbacks.begin() ; it != data.feedbacks.end(); ++it)
  {
    ab << it->current;
  }
  return ab;
}


//! Overload stream operator to easily deserialize all channel feedback data
inline icl_comm::ArrayBuilder& operator >> (icl_comm::ArrayBuilder& ab, S5FHControllerFeedbackAllChannels& data)
{
  // The Data is transmitted not channel by channel but rather position first, Currents afterwards for all channels
  for (std::vector<S5FHControllerFeedback>::iterator it = data.feedbacks.begin() ; it != data.feedbacks.end(); ++it)
  {
    ab >> it->position;
  }

  for (std::vector<S5FHControllerFeedback>::iterator it = data.feedbacks.begin() ; it != data.feedbacks.end(); ++it)
  {
    ab >> it->current;
  }
  return ab;
}

//! Output stream operator for easy output of all channel feedback data
inline std::ostream& operator << (std::ostream& o, const S5FHControllerFeedbackAllChannels& cf)
{
  o << "Feedbacks: " ;
  unsigned int i = 0;
  for (std::vector<S5FHControllerFeedback>::const_iterator it = cf.feedbacks.begin() ; it != cf.feedbacks.end(); ++it,++i)
  {
    o << "Chan " << i << " : "<< *it;
  }
  o << std::endl;
  return o;
}

}

#endif // S5FHCONTROLLERFEEDBACK_H
