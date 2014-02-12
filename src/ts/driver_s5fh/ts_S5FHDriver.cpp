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
#include <driver_s5fh/S5FHControlCommand.h>
#include <driver_s5fh/S5FHControllerFeedback.h>
#include <driver_s5fh/S5FHEncoderSettings.h>
#include <driver_s5fh/S5FHController.h>

using icl_comm::ArrayBuilder;
using namespace driver_s5fh;

BOOST_AUTO_TEST_SUITE(ts_S5FHDriver)

ArrayBuilder payload(40);

BOOST_AUTO_TEST_CASE(ConvertPosSettings)
{
  std::cout << "Conversion test of Position Settings ....";

  // Reset Array Builder
  payload.reset(40);

  // Create Structures
  S5FHPositionSettings test_pos_settings_in =  {0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.1};
  S5FHPositionSettings test_pos_settings_out;

  // Conversion
  payload << test_pos_settings_in;
  payload >> test_pos_settings_out;

  BOOST_CHECK_EQUAL(test_pos_settings_in,test_pos_settings_out);

  std::cout << "Done" << std::endl;
}

BOOST_AUTO_TEST_CASE(ConvertCurSettings)
{
  std::cout << "Conversion test of Current Settings ....";

  // Reset Array Builder
  payload.reset(40);

  // Create Structures
  S5FHCurrentSettings test_cur_settings_in =  {0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.1};
  S5FHCurrentSettings test_cur_settings_out;

  // Conversion
  payload << test_cur_settings_in;
  payload >> test_cur_settings_out;

  BOOST_CHECK_EQUAL(test_cur_settings_in,test_cur_settings_out);

  std::cout << "Done" << std::endl;
}

BOOST_AUTO_TEST_CASE(ConvertControlCommand)
{
  std::cout << "Conversion test of Controll Command ....";

  // Reset Array Builder
  payload.reset(40);

  // Create Structures
  S5FHControlCommand test_control_command_in(23);
  S5FHControlCommand test_control_command_out(0);

  // Conversion
  payload << test_control_command_in;
  payload >> test_control_command_out;

  BOOST_CHECK_EQUAL(test_control_command_in,test_control_command_out);

  std::cout << "Done" << std::endl;
}


BOOST_AUTO_TEST_CASE(ConvertControlFeedback)
{
  std::cout << "Conversion test of ControllerFeedback ....";

  // Reset Array Builder
  payload.reset(40);

  // Create Structures
  S5FHControllerFeedback test_controller_feedback_in(23,42);
  S5FHControllerFeedback test_controller_feedback_out;

  // Conversion
  payload << test_controller_feedback_in;
  payload >> test_controller_feedback_out;

  BOOST_CHECK_EQUAL(test_controller_feedback_in,test_controller_feedback_out);

  std::cout << "Done" << std::endl;
}

BOOST_AUTO_TEST_CASE(ConvertControllerState)
{
  std::cout << "Converstion test of ControllerState ....";

  // Reset Array Builder
  payload.reset(40);

  // Create Structures
  S5FHControllerState test_controller_state_in(0x001F,0x001F,0x0200,0x02000,0x0001,0x0001);
  S5FHControllerState test_controller_state_out;

  // Conversion
  payload << test_controller_state_in;
  payload >> test_controller_state_out;



  BOOST_CHECK_EQUAL(test_controller_state_in,test_controller_state_out);

  std::cout << "Done" << std::endl;
}

BOOST_AUTO_TEST_CASE(ConvertEncoderSettings)
{
  std::cout << "Converstion test of EncoderSettings ....";

  // Reset Array Builder
  payload.reset(40);

  // Create Structures
  S5FHEncoderSettings test_encoder_settings_in(23);
  S5FHEncoderSettings test_encoder_settings_out;

  // Conversion
  payload << test_encoder_settings_in;
  payload >> test_encoder_settings_out;

  BOOST_CHECK_EQUAL(test_encoder_settings_in,test_encoder_settings_out);

  std::cout << "Done" << std::endl;
}


BOOST_AUTO_TEST_CASE(ConvertSerialPacket)
{

  std::cout << "Converstion test of Serial Packet ....";

  // Reset Array Builder
  payload.reset(40);

  // Create Structures
  ArrayBuilder packet;
  S5FHSerialPacket test_serial_packet_in(40,5);
  S5FHSerialPacket test_serial_packet_out(40);
  S5FHPositionSettings test_pos_settings_in =  {0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.1};
  S5FHPositionSettings test_pos_settings_out;

  // Conversion
  payload << test_pos_settings_in;
  // Insertion
  test_serial_packet_in.data = payload.array;
  // Converstion (this would be done by serialinterface)
  packet << test_serial_packet_in;
  packet >> test_serial_packet_out;
  payload.reset(40);
  payload.appendWithoutConversion(test_serial_packet_out.data);
  payload >> test_pos_settings_out;

  BOOST_CHECK_EQUAL(test_serial_packet_in,test_serial_packet_out);
  BOOST_CHECK_EQUAL(test_pos_settings_in,test_pos_settings_out);

  std::cout << "Done" << std::endl;
}

BOOST_AUTO_TEST_CASE(ControllerreceiveFeedback)
{

  std::cout << "Controller receiving feedback Packet ....";

  // Initialize logging
  icl_core::logging::initialize();

  // Reset Array Builder
  // BEWARE OF ARRAY LENGTH!
  payload.reset(6);

  // Create Structures
  S5FHController controller;
  ArrayBuilder packet;
  S5FHCHANNEL channel = eS5FH_INDEX_FINGER_DISTAL;
  S5FHSerialPacket test_serial_packet(6,S5FH_SET_CONTROL_COMMAND|static_cast<u_int8_t>(channel << 4));
  S5FHControllerFeedback test_controller_feedback(23,42);
  // Conversion
  payload << test_controller_feedback;


  // Insertion
  test_serial_packet.data = payload.array;

  // Emulate received packet
  controller.receivedPacketCallback(test_serial_packet,1);

  // Get packet from controller
  S5FHControllerFeedback feedback_out;
  controller.getControllerFeedback(channel,feedback_out);

  BOOST_CHECK_EQUAL(test_controller_feedback,feedback_out);

  std::cout << "Done" << std::endl;
}


// Firmware is not working yet
//BOOST_AUTO_TEST_CASE(FirmWareOutput)
//{

//  S5FHFirmwareInfo firmware_info;
//  firmware_info.version_major = 23;
//  firmware_info.version_minor =42;
//  firmware_info.s5fh = "s5fh";
//  firmware_info.text = "FZI HAND CONTROL V1.1";

//  payload.reset(0);
//  payload << firmware_info;

//  std::cout << "Payload of FW:" << payload << std::endl;

//  S5FHFirmwareInfo firmware_info2;
//  payload >> firmware_info2;

//  std::cout << "FW1:" << firmware_info << std::endl;
//  std::cout << "FW2:" << firmware_info2 << std::endl;
//  BOOST_CHECK_EQUAL(firmware_info,firmware_info2);
//}



BOOST_AUTO_TEST_SUITE_END()
