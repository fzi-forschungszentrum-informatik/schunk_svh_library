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
#ifndef S5FHSERIALPACKET_H
#define S5FHSERIALPACKET_H

#include <icl_core/BaseTypes.h>
#include <icl_core/Vector.h>

namespace driver_s5fh {

//! packet size consts
const size_t cPACKET_APPENDIX_SIZE = 8;
const size_t cDEFAULT_PACKET_SIZE = 48;

//! packet headers
const uint8_t PACKET_HEADER1 = 0x4C;
const uint8_t PACKET_HEADER2 = 0xAA;

/*!
 * \brief The SerialPacket holds the (non generated) header and data of one message to the S5FH-Hardware
 */
struct S5FHSerialPacket
{
  //! Continiously incremented counter per package
  uint8_t index;
  //! Adress denotes the actual function of the package
  uint8_t address;
  //! Payload of the package
  icl_core::Vector <uint8_t> data;
};

}

#endif // S5FHSERIALPACKET_H
