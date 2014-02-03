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
#ifndef S5FHFIRMWAREINFO_H
#define S5FHFIRMWAREINFO_H

#include <icl_core/BaseTypes.h>
#include <icl_core/Vector.h>

namespace driver_s5fh {

/*!
 * \brief The S5FHFirmwareInfo  holds the data of a firmware response from the hardware
 */
struct S5FHFirmwareInfo
{
  //! 4 Char identifier
  uint8_t s5fh[4];
  //! Major version number
  uint16_t version_major;
  //! Minor version number
  uint16_t version_minor;
  //! 48 chars of text (free)
  uint8_t text[48];
};

}



#endif // S5FHFIRMWAREINFO_H
