// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Lars pfotzer
 * \date    2014-01-30
 *
 */
//----------------------------------------------------------------------
#ifndef DRIVER_S5FH_S5FH_DATA_STRUCTURES_H_INCLUDED
#define DRIVER_S5FH_S5FH_DATA_STRUCTURES_H_INCLUDED

#include <icl_core/BaseTypes.h>

namespace driver_s5fh {

struct SerialPacket
{
  uint8_t index;
  uint8_t address;
  uint8_t data[];
};

}

#endif
