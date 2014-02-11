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

//! adress constants for commands
const uint8_t S5FH_GET_CONTROL_FEEDBACK = 0x00;
const uint8_t S5FH_SET_CONTROL_COMMAND = 0x01;
const uint8_t S5FH_GET_POSITION_SETTINGS = 0x04;
const uint8_t S5FH_SET_POSITION_SETTINGS = 0x05;
const uint8_t S5FH_GET_CURRENT_SETTINGS = 0x06;
const uint8_t S5FH_SET_CURRENT_SETTINGS = 0x07;
const uint8_t S5FH_GET_CONTROLLER_STATE = 0x08;
const uint8_t S5FH_SET_CONTROLLER_STATE = 0x09;
const uint8_t S5FH_GET_ENCODER_VALUES = 0x0A;
const uint8_t S5FH_SET_ENCODER_VALUES = 0x0B;
const uint8_t S5FH_GET_FIRMWARE_INFO = 0x0C;

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
  S5FHSerialPacket(uint8_t _address = S5FH_GET_CONTROL_FEEDBACK,size_t _data_length=0):
    address(_address),
    data(_data_length,0)
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
