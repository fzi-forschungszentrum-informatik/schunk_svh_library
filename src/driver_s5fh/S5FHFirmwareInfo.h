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
 * This file contains the S5FHFirmwareInfo data structure that is used to
 * receive information about the current firmware settings
 */
//----------------------------------------------------------------------
#ifndef S5FHFIRMWAREINFO_H
#define S5FHFIRMWAREINFO_H

namespace driver_s5fh {

/*!
 * \brief The S5FHFirmwareInfo  holds the data of a firmware response from the hardware
 */
struct S5FHFirmwareInfo
{
  //! 4 bytes identifier
  std::string s5fh;
  //! Major version number
  uint16_t version_major;
  //! Minor version number
  uint16_t version_minor;
  //! 48 bytes! of text (free)
  std::string text;

  //! Compares two S5FHFirmware objects.
  bool operator == (const S5FHFirmwareInfo& other) const
  {
    return (version_major == other.version_major && version_minor == other.version_minor);
  }
};

//! overload stream operator to easily serialize firmware data
inline icl_comm::ArrayBuilder& operator << (icl_comm::ArrayBuilder& ab, S5FHFirmwareInfo& data)
{
  // Stream operator can not handle arrays (due to missing size information) to make things easy we just copy the data around. Feel free to do something else
  // Todo : Can the operator handle this without conversion? Also .. string uses char.. and not unsigned char.. but that is also 8 bit so.. we are save.. maybe we have to use std::basic_string<unsinge_char> ... try it..
  std::vector<uint8_t> text(48);
  std::vector<uint8_t> s5fh(4);
  s5fh.insert(s5fh.begin(),data.s5fh.begin(),data.s5fh.end());
  text.insert(text.begin(),data.text.begin(),data.text.end());

  ab << s5fh
     << data.version_major
     << data.version_minor
     << text;
  return ab;
}



//! overload stream operator to easily serialize firmware data
inline icl_comm::ArrayBuilder& operator >> (icl_comm::ArrayBuilder& ab, S5FHFirmwareInfo& data)
{
  // Stream operator can not handle arrays (due to missing size information) to make things easy we just copy the data around. Feel free to do something else
  std::vector<uint8_t> text(48);
  std::vector<uint8_t> s5fh(5);

  ab >> s5fh
     >> data.version_major
     >> data.version_minor
     >> text;


  data.text = std::string(text.begin(),text.end());
  data.s5fh = std::string(s5fh.begin(),s5fh.end());
//  std::copy (text.begin(),text.end(),data.text);
//  std::copy (s5fh.begin(),s5fh.end(),data.s5fh);

  return ab;
}

//! Output Stream operator for easy output of the firmware information
inline std::ostream& operator << (std::ostream& o, const S5FHFirmwareInfo& fw)
{
  o << fw.s5fh.c_str()  << " " << fw.version_major << "." << fw.version_minor << " : " << fw.text.c_str() << endl;
  return o;
}



}



#endif // S5FHFIRMWAREINFO_H
