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

using icl_comm::ArrayBuilder;
using namespace driver_s5fh;

BOOST_AUTO_TEST_SUITE(ts_S5FHDriver)

S5FHPositionSettings test_pos_settings =  {0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.1};
S5FHCurrentSettings test_cur_settings =  {0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.1};

std::vector<uint8_t> test_vector;
std::vector<uint16_t> test_vector2;

BOOST_AUTO_TEST_CASE(ConvertSettingsTest)
{
  // See if the conversion of position settings worked
  ArrayBuilder ab(40);
  ArrayBuilder ab2(40);
  ab << test_pos_settings;
  ab2 << test_cur_settings;
  std::cout << "S5FH Driver array builder test:" << std::endl;
  std::cout << "  Position Settings raw data: " << ab << std::endl;
  std::cout << "------------------------------" << std::endl;
  std::cout << "  Current Settings raw data: " << ab2 << std::endl;

  // Test if the Vectors Work with the ArrayBuilder
  test_vector.push_back(1);
  test_vector.push_back(2);
  test_vector.push_back(3);
  test_vector.push_back(4);
  ab << test_vector ;

  std::cout << "Testvector " << ab << std::endl;

  test_vector2.push_back(5);
  test_vector2.push_back(6);
  test_vector2.push_back(7);
  test_vector2.push_back(8);
  ab << test_vector2 ;

  std::cout << "Testvector2 " << ab << std::endl;

  // TODO: Make these to automated tests ;) and not just couts
}

BOOST_AUTO_TEST_CASE(ConvertSerialPacket)
{
  std::cout << std::endl;

  ArrayBuilder payload(40);
  ArrayBuilder packet;
  S5FHSerialPacket test_serial_packet;

  // Get the Position settings as payload
  payload << test_pos_settings;
  // Generate the header information
  test_serial_packet.address = 5; // Set Position settings
  test_serial_packet.index = 0;   //
  // Set the payload (converted array of position settings)
  test_serial_packet.data = payload.array;

  // serialize the Packet
  packet << test_serial_packet;

  std::cout << "The Serial packet with the adress: " << (int)test_serial_packet.address << " index: " << (int)test_serial_packet.index << "Length: " << test_serial_packet.data.size() << " Has the following RAW data: "<< std::endl;
  std::cout << "Packet: " << packet << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()
