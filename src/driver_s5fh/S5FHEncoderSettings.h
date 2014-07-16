// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Georg Heppner
 * \date    2014-02-11
 * \date    2014-07-16
 *
 * This file contains the EncoderSettings data structure that is used to
 * send and receive the settings for the encoders. Basically these are multipliers
 * and could be use to pre scale the encoders to meaningfull values (i.e. rad)
 * As this is done by this driver, sending them should never be necessary
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

  // TODO Provide a constructor that allows for seperate encoder settings in the hardware
  /*!
   * \brief S5FHEncoderSettings Default CTOR will assign 9x1 to the scalings if no argument is given
   * \param _scaling scaling to use for the encoders (everyone is scaled the same)
   */
  S5FHEncoderSettings(uint32_t _scaling = 1):
  scalings(9,_scaling)
  {}

  //! Compares two S5FHEncoderSettings objects.
  bool operator == (const S5FHEncoderSettings& other) const
  {
    return (scalings == other.scalings);
  }
};


//! overload stream operator to easily serialize encoder scaling data
inline icl_comm::ArrayBuilder& operator << (icl_comm::ArrayBuilder& ab, const S5FHEncoderSettings& data)
{
  // Trivial as the vector slicing is already done by the arraybuilder
  ab << data.scalings;
  return ab;
}

//! overload stream operator to easily serialize encoder scaling data
inline icl_comm::ArrayBuilder& operator >> (icl_comm::ArrayBuilder& ab, S5FHEncoderSettings& data)
{
  ab >> data.scalings;
  return ab;
}


//! Output Stream operator for easy output of the encoder scalings
inline std::ostream& operator << (std::ostream& o, const S5FHEncoderSettings& es)
{
  o << "Scalings: ";
  for (size_t i = 0; i < es.scalings.size(); i++)
  {
    o << (int)i << ":" <<es.scalings[i] << " ";
  }

  o << std::endl;
  return o;
}

}
#endif // S5FHENCODERSETTINGS_H
