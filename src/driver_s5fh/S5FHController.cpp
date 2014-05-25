// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Lars Pfotzer
 * \date    2014-01-30
 *
 */
//----------------------------------------------------------------------
#include "driver_s5fh/S5FHController.h"

#include "driver_s5fh/Logging.h"
#include <icl_comm/ByteOrderConversion.h>
#include <boost/bind/bind.hpp>

using icl_comm::ArrayBuilder;

/* FIXME:
 *  *
 * TODO:
 *
 *TODO(optional):
 * - Data about the positions and currents is currently pulled by the fingermanager -> this could be enhanced by using mutexes to inform higher layers about changes
 * - Sanity checks of set values to ensure safe access that will impose absolute hardware limits (i.e. CurrentSettings)
 * - Logging Output!!! Everywhere!
 * - bool isEnabled(const S5FHCHANNEL &channel); --> Check if the finger was actually turned on by evaluating (PWM_RESET & (1 << Channel))
 * - Do somethings usefull with the FirmwareInformation
 */

namespace driver_s5fh {


//! Description for enum matching:
const char * S5FHController::m_channel_description[]= {
  "Thumb_Flexion",
  "Thumb_Opposition", // wrist
  "Index_Finger_Distal",
  "Index_Finger_Proximal",
  "Middle_Finger_Distal",
  "Middle_Finger_Proximal",
  "Ring_Finger",
  "Pinky",
  "Finger_Spread",
  NULL
};

S5FHController::S5FHController():
  m_current_settings(eS5FH_DIMENSION,S5FHCurrentSettings()),  // Vectors have to be filled with objects for correct deserialization
  m_position_settings(eS5FH_DIMENSION,S5FHPositionSettings()),
  m_controller_feedback(eS5FH_DIMENSION,S5FHControllerFeedback()),
  m_serial_interface(new S5FHSerialInterface(boost::bind(&S5FHController::receivedPacketCallback,this,_1,_2))),
  m_enable_mask(0),
  m_received_package_count(0)
{


}

S5FHController::~S5FHController()
{
  if (m_serial_interface != NULL)
  {
    disconnect();
    delete m_serial_interface;
    m_serial_interface = NULL;
  }

}

bool S5FHController::connect(const std::string &dev_name)
{
  if (m_serial_interface != NULL)
    return m_serial_interface->connect(dev_name);
  else
    return false;
}

void S5FHController::disconnect()
{
  // Disable all channels
  disableChannel(eS5FH_ALL);

  m_serial_interface->close();
}

void S5FHController::setControllerTarget(const S5FHCHANNEL& channel, const u_int32_t& position)
{
  // TODO Opt: Sanity checks for position
  if (channel != eS5FH_ALL)
  {
    S5FHSerialPacket serial_packet(0,S5FH_SET_CONTROL_COMMAND|static_cast<u_int8_t>(channel << 4));
    S5FHControlCommand control_command(position);
    // Note the 40 byte ArrayBuilder initialization -> this is needed to get a zero padding in the serialpacket. Otherwise it would be shorter
    ArrayBuilder ab(40);
    ab << control_command;
    serial_packet.data = ab.array;
    m_serial_interface ->sendPacket(serial_packet);

    LOGGING_DEBUG_C(DriverS5FH, S5FHController, "Control command was given for channel: "<< channel << "Driving motor to position: "<< position << endl);
  }
  else
  {
    LOGGING_WARNING_C(DriverS5FH, S5FHController, "Control command was given for unknown channel: "<< channel << "- ignoring request"<< endl);
  }
}

void S5FHController::setControllerTargetAllChannels(const std::vector<int32_t> &positions)
{
  if (positions.size() >= eS5FH_DIMENSION)
  {
   S5FHSerialPacket serial_packet(0,S5FH_SET_CONTROL_COMMAND_ALL);
   S5FHControlCommandAllChannels control_command(positions);
   ArrayBuilder ab(40);
   ab << control_command;
   serial_packet.data = ab.array;
   m_serial_interface ->sendPacket(serial_packet);

   LOGGING_TRACE_C(DriverS5FH, S5FHController, "Control command was given for all channels: Driving motors to positions: "<< positions[0] << " , " << positions[1] << " , " << positions[2] << " , " << positions[3] << " , " << positions[4] << " , " << positions[5] << " , " << positions[6] << " , " << positions[7] << " , " << positions[8] << " , " << endl);
  }
  else
  {
    LOGGING_ERROR_C(DriverS5FH, S5FHController, "Control command was given for all channels but with to few points. Expected at least "<< eS5FH_DIMENSION << " values but only got " << positions.size());
  }
}

void S5FHController::enableChannel(const S5FHCHANNEL &channel)
{
  S5FHSerialPacket serial_packet(0,S5FH_SET_CONTROLLER_STATE);
  S5FHControllerState controller_state;
  ArrayBuilder ab(40);

  //TODO: make the following better.. for now the MeCoVis example is just copied more or less exactly
  if (m_enable_mask == 0)
  {
    LOGGING_DEBUG_C(DriverS5FH, S5FHController, "Enable was called and no channel was previously activated, commands are sent individually......" << endl);
    LOGGING_DEBUG_C(DriverS5FH, S5FHController, "Sending pwm_fault and pwm_otw..." << endl);
    // reset????
    controller_state.pwm_fault = 0x001F;
    controller_state.pwm_otw   = 0x001F;
    ab << controller_state;
    serial_packet.data = ab.array;
    m_serial_interface ->sendPacket(serial_packet);
    ab.reset(40);

    icl_core::os::usleep(2000);

    LOGGING_DEBUG_C(DriverS5FH, S5FHController, "Adding pwm_reset and pwm_active..." << endl);
    // enable +12v supply driver ??????
    controller_state.pwm_reset = 0x0200;
    controller_state.pwm_active = 0x0200;
    ab << controller_state;
    serial_packet.data = ab.array;
    m_serial_interface ->sendPacket(serial_packet);
    ab.reset(40);

    icl_core::os::usleep(2000);

     LOGGING_DEBUG_C(DriverS5FH, S5FHController, "Adding pos_ctrl and cur_ctrl..." << endl);
    // enable controller ???????
    controller_state.pos_ctrl = 0x0001;
    controller_state.cur_ctrl = 0x0001;
    ab << controller_state;
    serial_packet.data = ab.array;
    m_serial_interface ->sendPacket(serial_packet);
    ab.reset(40);

     icl_core::os::usleep(2000);

    LOGGING_DEBUG_C(DriverS5FH, S5FHController, "...Done" << endl);
  }

  // FIXME!!! WARNING CHECK WHAT MeCoVIS is using for their channels as this might have very serious influences!!!!
  if (channel >=0 && channel < eS5FH_DIMENSION)
  {
    LOGGING_DEBUG_C(DriverS5FH, S5FHController, "Enabling motor: "<< channel << endl);
    m_enable_mask |= (1<<channel);

    // SENDING ALL AT ONCE WILL LEAD TO "Jumping" behaviour of the controller on faster Systems ---> MeCoVis hardware problem .. ask about that
//    controller_state.pwm_fault  = 0x001F;
//    controller_state.pwm_otw    = 0x001F;
//    controller_state.pwm_reset  = (0x0200 | (m_enable_mask & 0x01FF));
//    controller_state.pwm_active = (0x0200 | (m_enable_mask & 0x01FF));
//    controller_state.pos_ctrl   = 0x0001;
//    controller_state.cur_ctrl   = 0x0001;
    ab.reset(40);
    controller_state.pwm_fault = 0x001F;
    controller_state.pwm_otw   = 0x001F;
    controller_state.pwm_reset = (0x0200 | (m_enable_mask & 0x01FF));
    controller_state.pwm_active =(0x0200 | (m_enable_mask & 0x01FF));
    ab << controller_state;
    serial_packet.data = ab.array;
    m_serial_interface ->sendPacket(serial_packet);
    ab.reset(40);

    // WARNING: DO NOT ! REMOVE THESE DELAYS OR THE HARDWARE WILL! FREAK OUT!
    icl_core::os::usleep(500);

    controller_state.pos_ctrl = 0x0001;
    controller_state.cur_ctrl = 0x0001;
    ab << controller_state;
    serial_packet.data = ab.array;
    m_serial_interface ->sendPacket(serial_packet);
    ab.reset(40);

    // WARNING: DO NOT ! REMOVE THESE DELAYS OR THE HARDWARE WILL! FREAK OUT!
    icl_core::os::usleep(500);

    ab << controller_state;
    serial_packet.data = ab.array;
    m_serial_interface ->sendPacket(serial_packet);
  }
  else
  {
    LOGGING_ERROR_C(DriverS5FH, S5FHController, "Activation request for ALL or unknown channel: "<< channel << "- ignoring request"<< endl);
  }

}


void S5FHController::disableChannel(const S5FHCHANNEL& channel)
{
  if (m_serial_interface != NULL && m_serial_interface->isConnected())
  {
    S5FHSerialPacket serial_packet(0,S5FH_SET_CONTROLLER_STATE);
    S5FHControllerState controller_state;
    ArrayBuilder ab(40);
    //TODO: The following code is very.. strange -> Ask MeCoVis if that is really... really really what one is supposed to send to disable things
    if (channel == eS5FH_ALL)
    {
      m_enable_mask = 0;
      controller_state.pwm_fault = 0x001F;
      controller_state.pwm_otw   = 0x001F;

      ab << controller_state;
      serial_packet.data = ab.array;
      m_serial_interface->sendPacket(serial_packet);
    }
    else if (channel >=0 && channel < eS5FH_DIMENSION)
    {
      controller_state.pwm_fault = 0x001F;
      controller_state.pwm_otw   = 0x001F;
      //Disable the finger in the bitmask
      m_enable_mask &= ~(1<<channel);

      if (m_enable_mask != 0) // pos and current control stay on then
      {
        controller_state.pwm_reset  = (0x0200 | (m_enable_mask & 0x01FF));
        controller_state.pwm_active = (0x0200 | (m_enable_mask & 0x01FF));
        controller_state.pos_ctrl   = 0x0001;
        controller_state.cur_ctrl   = 0x0001;
      }

      ab << controller_state;
      serial_packet.data = ab.array;
      m_serial_interface->sendPacket(serial_packet);

    }
    else
    {
      LOGGING_WARNING_C(DriverS5FH, S5FHController, "Disable was requestet for unknown channel: "<< channel << "- ignoring request" << endl);
    }
  }
  else
  {
    LOGGING_ERROR_C(DriverS5FH, S5FHController, "Serial interface is not connected!" << endl);
  }
}


void S5FHController::requestControllerState()
{
  S5FHSerialPacket serial_packet(40,S5FH_GET_CONTROLLER_STATE);
  m_serial_interface ->sendPacket(serial_packet);
}

void S5FHController::requestControllerFeedback(const S5FHCHANNEL& channel)
{
  if ((channel != eS5FH_ALL) && (channel >=0 && channel < eS5FH_DIMENSION))
  {
    S5FHSerialPacket serial_packet(40,S5FH_GET_CONTROL_FEEDBACK|static_cast<u_int8_t>(channel << 4));
    m_serial_interface ->sendPacket(serial_packet);

    LOGGING_DEBUG_C(DriverS5FH, S5FHController, "Controller feedback was requested for channel: "<< channel << endl);

  }
  else
  {
    LOGGING_WARNING_C(DriverS5FH, S5FHController, "Controller feedback was requestet for unknown channel: "<< channel << "- ignoring request"<< endl);
  }

}

void S5FHController::requestControllerFeedbackAllChannels()
{
   S5FHSerialPacket serial_packet(40,S5FH_GET_CONTROL_FEEDBACK_ALL);
   m_serial_interface ->sendPacket(serial_packet);

   LOGGING_DEBUG_C(DriverS5FH, S5FHController, "Controller feedback was requested for all channels " << endl);
}


void S5FHController::requestPositionSettings(const S5FHCHANNEL& channel)
{
  S5FHSerialPacket serial_packet((S5FH_GET_POSITION_SETTINGS| static_cast<u_int8_t>(channel << 4)),40);
  m_serial_interface ->sendPacket(serial_packet);
}


void S5FHController::setPositionSettings(const S5FHCHANNEL& channel,const S5FHPositionSettings& position_settings)
{
  if ((channel != eS5FH_ALL) && (channel >=0 && channel < eS5FH_DIMENSION))
  {
    S5FHSerialPacket serial_packet(0,S5FH_SET_POSITION_SETTINGS|static_cast<u_int8_t>(channel << 4));
    ArrayBuilder ab;
    ab << position_settings;
    serial_packet.data = ab.array;
    m_serial_interface ->sendPacket(serial_packet);

    // Save already in case we dont get imeediate response
    m_position_settings[channel] = position_settings;

    LOGGING_DEBUG_C(DriverS5FH, S5FHController, "Position controller settings where send to change channel: "<< channel << endl);

  }
  else
  {
    LOGGING_WARNING_C(DriverS5FH, S5FHController, "Position controller settings where given for unknown channel: "<< channel << "- ignoring request"<< endl);
  }
}


void S5FHController::requestCurrentSettings(const S5FHCHANNEL& channel)
{
  if (channel != eS5FH_ALL)
  {
    S5FHSerialPacket serial_packet(40,(S5FH_GET_CURRENT_SETTINGS|static_cast<u_int8_t>(channel << 4)));
    m_serial_interface ->sendPacket(serial_packet);
  }
  else
  {
    LOGGING_WARNING_C(DriverS5FH, S5FHController, "Get Current Settings can only be requested with a specific channel, ALL was selected " << endl);
  }

}


void S5FHController::setCurrentSettings(const S5FHCHANNEL& channel,const S5FHCurrentSettings& current_settings)
{
  if ((channel != eS5FH_ALL) && (channel >=0 && channel < eS5FH_DIMENSION))
  {
    S5FHSerialPacket serial_packet(0,S5FH_SET_CURRENT_SETTINGS|static_cast<u_int8_t>(channel << 4));
    ArrayBuilder ab;
    ab << current_settings;
    serial_packet.data = ab.array;
    m_serial_interface ->sendPacket(serial_packet);

    // Save already in case we dont get imediate response
    m_current_settings[channel] = current_settings;

    LOGGING_DEBUG_C(DriverS5FH, S5FHController, "Current controller settings where send to change channel: "<< channel << endl);

  }
  else
  {
    LOGGING_WARNING_C(DriverS5FH, S5FHController, "Current controller settings where given for unknown channel: "<< channel << "- ignoring request"<< endl);
  }
}


void S5FHController::requestEncoderValues()
{
  S5FHSerialPacket serial_packet(40,S5FH_GET_ENCODER_VALUES);
  m_serial_interface ->sendPacket(serial_packet);
}

void S5FHController::setEncoderValues(const S5FHEncoderSettings &encoder_settings)
{
  S5FHSerialPacket serial_packet(0,S5FH_SET_ENCODER_VALUES);
  ArrayBuilder ab;
  ab << encoder_settings;
  serial_packet.data = ab.array;
  m_serial_interface ->sendPacket(serial_packet);

  // Save already in case we dont get imediate response
  m_encoder_settings = encoder_settings;

}


void S5FHController::requestFirmwareInfo()
{
  S5FHSerialPacket serial_packet(40,S5FH_GET_FIRMWARE_INFO);
  m_serial_interface->sendPacket(serial_packet);
}


void S5FHController::receivedPacketCallback(const S5FHSerialPacket& packet, unsigned int packet_count)
{
  // Extract Channel
  u_int8_t channel = (packet.address >> 4 ) & 0x0F;
  // Prepare Data for conversion
  ArrayBuilder ab;
  ab.appendWithoutConversion(packet.data);

  S5FHControllerFeedbackAllChannels feedback_all;

  m_received_package_count = packet_count;

  // Packet meaning is encoded in the lower nibble of the adress byte
  switch (packet.address & 0x0F)
  {
    case S5FH_GET_CONTROL_FEEDBACK:
    case S5FH_SET_CONTROL_COMMAND:
      if (channel >=0 && channel < eS5FH_DIMENSION)
      {
        //std::cout << "Recieved: Controllerfeedback RAW Data: " << ab;
        ab >> m_controller_feedback[channel];
        LOGGING_TRACE_C(DriverS5FH, S5FHController, "Received a Control Feedback/Control Command packet for channel "<< channel << " Position: "<< (int)m_controller_feedback[channel].position  << " Current: "<< (int)m_controller_feedback[channel].current << endl);
      }
      else
      {
        LOGGING_ERROR_C(DriverS5FH, S5FHController, "Received a Control Feedback/Control Command packet for ILLEGAL channel "<< channel << "- packet ignored!" << endl);
      }
      break;
    case S5FH_GET_CONTROL_FEEDBACK_ALL:
    case S5FH_SET_CONTROL_COMMAND_ALL:
      // We cannot just read them al into the vector (which would have been nice) because the feedback of all channels is structured
      // Different from the feedback of one channel. So the S5FHControllerFeedbackAllChannels is used as an intermediary ( handles the deserialization)
      ab >> feedback_all;
      m_controller_feedback = feedback_all.feedbacks;
      LOGGING_TRACE_C(DriverS5FH, S5FHController, "Received a Control Feedback/Control Command packet for channel all channels "<<  endl);
      break;
    case S5FH_GET_POSITION_SETTINGS:
    case S5FH_SET_POSITION_SETTINGS:
      if (channel >=0 && channel < eS5FH_DIMENSION)
      {
        //std::cout << "Recieved: Postitionsettings RAW Data: " << ab;
        ab >> m_position_settings[channel];
        LOGGING_TRACE_C(DriverS5FH, S5FHController, "Received a get/set position setting packet for channel "<< channel  << endl);
        LOGGING_TRACE_C(DriverS5FH, S5FHController, "wmn " << m_position_settings[channel].wmn << " "<< "wmx " << m_position_settings[channel].wmx << " "<< "dwmx "<< m_position_settings[channel].dwmx << " "<< "ky "  << m_position_settings[channel].ky  << " "<< "dt "  << m_position_settings[channel].dt  << " "<< "imn " << m_position_settings[channel].imn << " "<< "imx " << m_position_settings[channel].imx << " " << "kp "  << m_position_settings[channel].kp  << " " << "ki "  << m_position_settings[channel].ki  << " " << "kd "  << m_position_settings[channel].kd  << endl);

      }
      else
      {
        LOGGING_ERROR_C(DriverS5FH, S5FHController, "Received a get/set position setting packet for ILLEGAL channel "<< channel << "- packet ignored!" << endl);
      }
      break;
    case S5FH_GET_CURRENT_SETTINGS:
    case S5FH_SET_CURRENT_SETTINGS:
      if (channel >=0 && channel < eS5FH_DIMENSION)
      {
        //std::cout << "Recieved: Current Settings RAW Data: " << ab;
        ab >> m_current_settings[channel];
        LOGGING_TRACE_C(DriverS5FH, S5FHController, "Received a get/set current setting packet for channel "<< channel << endl);
        LOGGING_TRACE_C(DriverS5FH, S5FHController, "wmn "<< m_current_settings[channel].wmn << " " << "wmx "<< m_current_settings[channel].wmx << " " << "ky " << m_current_settings[channel].ky  << " " << "dt " << m_current_settings[channel].dt  << " " << "imn "<< m_current_settings[channel].imn << " " << "imx "<< m_current_settings[channel].imx << " "                   << "kp " << m_current_settings[channel].kp  << " " << "ki " << m_current_settings[channel].ki  << " " << "umn "<< m_current_settings[channel].umn << " " << "umx "<< m_current_settings[channel].umx << " "<< endl);
      }
      else
      {
        LOGGING_ERROR_C(DriverS5FH, S5FHController, "Received a get/set current setting packet for ILLEGAL channel "<< channel << "- packet ignored!" << endl);
      }
      break;
    case S5FH_GET_CONTROLLER_STATE:
    case S5FH_SET_CONTROLLER_STATE:
        //std::cout << "Recieved: Controller State RAW Data: " << ab;
        ab >> m_controller_state;
        //std::cout << "Received controllerState interpreded data: "<< m_controller_state << std::endl;
        LOGGING_TRACE_C(DriverS5FH, S5FHController, "Received a get/set controler state packet " << endl);
        LOGGING_TRACE_C(DriverS5FH, S5FHController, "Controllerstate (NO HEX):" << "pwm_fault " << "0x" << static_cast<int>(m_controller_state.pwm_fault) << " " << "pwm_otw " << "0x" << static_cast<int>(m_controller_state.pwm_otw) << " "  << "pwm_reset " << "0x" << static_cast<int>(m_controller_state.pwm_reset) << " " << "pwm_active " << "0x" << static_cast<int>(m_controller_state.pwm_active) << " " << "pos_ctr " << "0x" <<  static_cast<int>(m_controller_state.pos_ctrl) << " " << "cur_ctrl " << "0x" << static_cast<int>(m_controller_state.cur_ctrl) << endl);
      break;
    case S5FH_GET_ENCODER_VALUES:
    case S5FH_SET_ENCODER_VALUES:
        LOGGING_TRACE_C(DriverS5FH, S5FHController, "Received a get/set encoder settings packet " << endl);
        ab >> m_encoder_settings;
      break;
    case S5FH_GET_FIRMWARE_INFO:
        //std::cout << "Recieved: Firmware Settings RAW Data: " << ab;
        ab >> m_firmware_info;
        std::cout << "Received Firmware intepreted data: "<< m_firmware_info << std::endl;
        LOGGING_INFO_C(DriverS5FH, S5FHController, "Received a firmware packet" << endl);
        LOGGING_INFO_C(DriverS5FH, S5FHController, m_firmware_info.s5fh  << " " << m_firmware_info.version_major << "." << m_firmware_info.version_minor << " : " << m_firmware_info.text << endl);
      break;
    default:
        LOGGING_ERROR_C(DriverS5FH, S5FHController, "Received a Packet with unknown address: "<< (packet.address & 0x0F) << " - ignoring packet" << endl);
      break;
  }

}

bool S5FHController::getControllerFeedback(const S5FHCHANNEL &channel,S5FHControllerFeedback& controller_feedback)
{
  if(channel >= 0 && static_cast<u_int8_t>(channel) < m_controller_feedback.size())
  {
    controller_feedback = m_controller_feedback[channel];
    return true;
  }
  else
  {
    LOGGING_WARNING_C(DriverS5FH, S5FHController, "Feedback was requested for unknown channel: "<< channel<< "- ignoring request" << endl);
    return false;
  }
}

void S5FHController::getControllerFeedbackAllChannels(S5FHControllerFeedbackAllChannels& controller_feedback)
{
  controller_feedback.feedbacks = m_controller_feedback;
}

bool S5FHController::getPositionSettings(const S5FHCHANNEL &channel, S5FHPositionSettings &position_settings)
{
  if(channel >= 0 && static_cast<u_int8_t>(channel) < m_position_settings.size())
  {
    position_settings = m_position_settings[channel];
    return true;
  }
  else
  {
    LOGGING_WARNING_C(DriverS5FH, S5FHController, "Position settings were requested for unknown channel: "<< channel<< "- ignoring request" << endl);
    return false;
  }
}

bool S5FHController::getCurrentSettings(const S5FHCHANNEL &channel, S5FHCurrentSettings &current_settings)
{
  if(channel >= 0 && static_cast<u_int8_t>(channel) < m_current_settings.size())
  {
    current_settings = m_current_settings[channel];
    return true;
  }
  else
  {
    LOGGING_WARNING_C(DriverS5FH, S5FHController, "Current settings were requested for unknown channel: "<< channel<< "- ignoring request" << endl);
    return false;
  }

}


S5FHFirmwareInfo S5FHController::getFirmwareInfo()
{
  return m_firmware_info;
}

unsigned int S5FHController::getSentPackageCount()
{
  if (m_serial_interface != NULL)
  {
    return m_serial_interface->transmittedPacketCount();
  }
  else
  {
    LOGGING_WARNING_C(DriverS5FH, S5FHController, "Request for transmit packet count could not be answered as the device is not connectd - ignoring request" << endl);
    return 0;
  }
}

unsigned int S5FHController::getReceivedPackageCount()
{
  return m_received_package_count;
}

bool S5FHController::isEnabled(const S5FHCHANNEL &channel)
{
  return ((1 << channel & m_enable_mask) > 0);
}




}
