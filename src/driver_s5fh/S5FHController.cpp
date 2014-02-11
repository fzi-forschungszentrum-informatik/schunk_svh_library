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

namespace driver_s5fh {

using icl_comm::ArrayBuilder;

S5FHController::S5FHController(const std::string& serial_dev_name):
  m_serial_interface(new S5FHSerialInterface(serial_dev_name,boost::bind(&S5FHController::receivedPacketCallback,this,_1,_2)))
{
}

S5FHController::~S5FHController()
{
}


void S5FHController::setControllerTarget(const S5FHCHANNEL& channel, const u_int32_t& position)
{


}

void S5FHController::enableChannel(const S5FHCHANNEL &channel)
{

}


void S5FHController::disableChannel(const S5FHCHANNEL& channel)
{

}


void S5FHController::requestControllerFeedback(const S5FHCHANNEL& channel)
{

}


void S5FHController::requestPositionSettings(const S5FHCHANNEL& channel)
{
  S5FHSerialPacket serial_packet((S5FH_GET_POSITION_SETTINGS| static_cast<u_int8_t>(channel << 4)),40);
  m_serial_interface ->sendPacket(serial_packet);
}


void S5FHController::setPositionSettings(const S5FHCHANNEL& channel,const S5FHPositionSettings& position_settings)
{
   //m_position_settings[channel] = position_settings;
}


void S5FHController::requestCurrentSettings(const S5FHCHANNEL& channel)
{
  if (channel != eS5FH_ALL)
  {
    S5FHSerialPacket serial_packet((S5FH_GET_CURRENT_SETTINGS|static_cast<u_int8_t>(channel << 4)),40);
    m_serial_interface ->sendPacket(serial_packet);
  }
  else
  {
    LOGGING_WARNING_C(DriverS5FH, S5FHController, "Get Current Settings can only be requested with a specific channel, ALL was selected " << endl);
  }

}


void S5FHController::setCurrentSettings(const S5FHCHANNEL& channel,const S5FHCurrentSettings& current_settings)
{
     // m_current_settings[channel] = current_settings;
}


void S5FHController::getEncoderValues()
{
  S5FHSerialPacket serial_packet(S5FH_GET_ENCODER_VALUES,40);
  m_serial_interface ->sendPacket(serial_packet);
}

void S5FHController::setEncoderValues(const S5FHEncoderSettings &encoder_settings)
{
  S5FHSerialPacket serial_packet(S5FH_SET_ENCODER_VALUES);
  ArrayBuilder ab;
  ab << encoder_settings;
  serial_packet.data = ab.array;
  m_serial_interface ->sendPacket(serial_packet);
}


void S5FHController::getFirmwareInfo()
{
  S5FHSerialPacket serial_packet(S5FH_GET_FIRMWARE_INFO,40);
  m_serial_interface->sendPacket(serial_packet);
}


void S5FHController::receivedPacketCallback(const S5FHSerialPacket& packet, unsigned int packet_count)
{
  // Todo: 1.Switch case to check what we got back
  // Todo: 2.Safe data in corresponding channel settings

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
    LOGGING_WARNING_C(DriverS5FH, S5FHController, "Feedback was requested for unknown channel: "<< channel << endl);
    return false;
  }
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
    LOGGING_WARNING_C(DriverS5FH, S5FHController, "Position settings were requested for unknown channel: "<< channel << endl);
    return false;
  }
}

bool S5FHController::getCurrentSettings(const S5FHCHANNEL &channel, S5FHPositionSettings &position_settings)
{
  if(channel >= 0 && static_cast<u_int8_t>(channel) < m_current_settings.size())
  {
    position_settings = m_position_settings[channel];
    return true;
  }
  else
  {
    LOGGING_WARNING_C(DriverS5FH, S5FHController, "Current settings were requested for unknown channel: "<< channel << endl);
    return false;
  }

}







}
