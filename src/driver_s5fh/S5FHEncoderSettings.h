// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Georg Heppner
 * \date    2014-02-11
 *
 */
//----------------------------------------------------------------------
#ifndef S5FHENCODERSETTINGS_H
#define S5FHENCODERSETTINGS_H


namespace driver_s5fh {

/*!
 * \brief The S5FHEncoderSettings hold the settings for the encoder scaling of each channel
 */
struct S5FHEncoderSettings {

  //! encoderSettings consist of multipliers for each encoder
  std::vector<uint32_t> scalings;

};


//! overload stream operator to easily serialize data
inline icl_comm::ArrayBuilder& operator << (icl_comm::ArrayBuilder& ab, const S5FHEncoderSettings& data)
{
  // Trivial as the vector slicing is already done by the arraybuilder
  ab << data.scalings;
  return ab;
}

//! overload stream operator to easily serialize data
inline icl_comm::ArrayBuilder& operator >> (icl_comm::ArrayBuilder& ab, S5FHEncoderSettings& data)
{
  ab >> data.scalings;
  return ab;
}







}

#endif // S5FHENCODERSETTINGS_H
