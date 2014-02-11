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

namespace driver_s5fh {

S5FHController::S5FHController()
{
}

S5FHController::~S5FHController()
{
}


void S5FHController::setControllerTarget(const size_t& channel, const u_int32_t& position)
{


}

void S5FHController::enableChannel(const size_t &channel)
{

}


void S5FHController::disableChannel(const size_t& channel)
{

}


void S5FHController::getControllerFeedback(const size_t& channel)
{

}


void S5FHController::getPositionSettings(const size_t& channel)
{

}


void S5FHController::setPositionSettings(const size_t& channel,const S5FHPositionSettings& position_settings)
{
   //m_position_settings[channel] = position_settings;
}


void S5FHController::getCurrentSettings(const size_t& channel)
{

}


void S5FHController::setCurrentSettings(const size_t& channel,const S5FHCurrentSettings& current_settings)
{
     // m_current_settings[channel] = current_settings;
}


void S5FHController::getEncoderValues()
{

}

//TODO: EncoderValues Type
//void setEncoderValues

void S5FHController::getFirmwareInfo()
{

}


ReceivedPacketCallback S5FHController::receivedPacketCallback(const S5FHSerialPacket& packet, unsigned int packet_count)
{

}







}
