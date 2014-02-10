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
#include <iostream>
#include <iomanip>

#include <icl_comm/ByteOrderConversion.h>

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
  std::vector <uint8_t> data;

  //!
  //! \brief S5FHSerialPacket contains the send and received data in raw format (bytewise)
  //! \param data_length initial size to set the data length to. NOTE: To Deserialize a packet this value HAS TO BE SET!
  //!
  S5FHSerialPacket(size_t data_length=0):
    data(data_length,0)
  {
  }

  //! Compares two S5FHSerialPackets objects.
  bool operator == (const S5FHSerialPacket& other) const
  {
    return
      (index == other.index
       && address == other.address
       && data == other.data);
  }
};

//! overload stream operator to easily serialize data
icl_comm::ArrayBuilder& operator << (icl_comm::ArrayBuilder& ab, const S5FHSerialPacket& data);

//! overload stream operator to easily Deserialize data
icl_comm::ArrayBuilder& operator >> (icl_comm::ArrayBuilder& ab, S5FHSerialPacket& data);

//! Output Stream operator
std::ostream& operator << (std::ostream& o, const S5FHSerialPacket& sp);


}

#endif // S5FHSERIALPACKET_H
