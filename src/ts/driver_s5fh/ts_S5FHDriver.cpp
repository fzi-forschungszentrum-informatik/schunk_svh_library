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
#include <icl_comm/ByteOrderConversion.h>
#include <boost/test/unit_test.hpp>

#include <driver_s5fh/S5FHPositionSettings.h>
#include <driver_s5fh/S5FHCurrentSettings.h>
#include <driver_s5fh/S5FHSerialPacket.h>
#include <driver_s5fh/S5FHControllerState.h>

using icl_comm::ArrayBuilder;
using namespace driver_s5fh;

BOOST_AUTO_TEST_SUITE(ts_S5FHDriver)

BOOST_AUTO_TEST_CASE(ConvertSettingsTest)
{
  S5FHPositionSettings test_pos_settings =  {0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.1};

  // See if the conversion of position settings worked
  ArrayBuilder ab(40);
  ab << test_pos_settings;
  std::cout << "S5FH Driver array builder test:" << std::endl;
  std::cout << "  Position Settings raw data: " << ab << std::endl;
  std::cout << "=================================================="<<std::endl<<std::endl;

}

BOOST_AUTO_TEST_CASE(ConvertSerialPacket)
{
  //std::cout << "Testing Conversion of Serial Packet" << std::endl;


  ArrayBuilder payload(40);
  ArrayBuilder packet;
  S5FHSerialPacket test_serial_packet;
  S5FHPositionSettings test_pos_settings =  {0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.1};

  // Get the Position settings as payload
  payload << test_pos_settings;
  // Generate the header information
  test_serial_packet.address = 5; // Set Position settings
  test_serial_packet.index = 0;   //
  // Set the payload (converted array of position settings)
  test_serial_packet.data = payload.array;

  // serialize the Packet
  packet << test_serial_packet;

  //std::cout << "Serial packet after transformation into byte array: " << test_serial_packet;

  // Create the structure to read in the packet
  S5FHSerialPacket test_serial_packet_in(40);
  // De-Serialize
  packet >> test_serial_packet_in;

  BOOST_CHECK_EQUAL(test_serial_packet,test_serial_packet_in);
}

BOOST_AUTO_TEST_CASE(ConvertControllerState)
{
  ArrayBuilder payload;
  S5FHControllerState controllerstate = {0x001F,0x001F,0x0200,0x0200,0x0001,0x0001};
  S5FHControllerState controllerstate_out;
    // Convert to byte Stream
  payload << controllerstate;
    // convert back
  payload >> controllerstate_out;

  BOOST_CHECK_EQUAL(controllerstate,controllerstate_out);
}


BOOST_AUTO_TEST_SUITE_END()
