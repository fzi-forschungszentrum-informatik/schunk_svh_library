// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Lars Pfotzer
 * \author  Georg Heppner
 * \date    2014-01-30
 *
 */
//----------------------------------------------------------------------
#ifndef DRIVER_S5FH_S5FH_CONTROLLER_H_INCLUDED
#define DRIVER_S5FH_S5FH_CONTROLLER_H_INCLUDED

#include <driver_s5fh/ImportExport.h>
#include <driver_s5fh/S5FHSerialInterface.h>
#include <driver_s5fh/S5FHReceiveThread.h>
#include <driver_s5fh/S5FHControlCommand.h>
#include <driver_s5fh/S5FHControllerFeedback.h>
#include <driver_s5fh/S5FHCurrentSettings.h>
#include <driver_s5fh/S5FHFirmwareInfo.h>
#include <driver_s5fh/S5FHPositionSettings.h>
#include <driver_s5fh/S5FHControllerState.h>

namespace driver_s5fh {

/*! This class controls the the SCHUNK five finger hand.
 */
class DRIVER_S5FH_IMPORT_EXPORT S5FHController
{
public:
  /*! Constructs a controller class for the SCHUNK five finger hand.
   */
  S5FHController(const std::string& serial_dev_name);

  /*! SCHUNK five finger hand destructor
   *  Disable the serial device and shut down hand as far
   *  as possible
   */
  ~S5FHController();

  /*!
   * \brief setting new position controller target for finger index
   * \param channel Motorchanel to set the target for
   * \param position Target position given in encoder Ticks
   */
  void setControllerTarget(const size_t& channel, const u_int32_t& position);


  // Access functions
  //!
  //! \brief Enable one or all motor channels
  //! \param channel Motor to activate 0 for all
  //!
  void enableChannel(const size_t& channel);

  //!
  //! \brief Disable one or all motor channels
  //! \param channel Motor to deactivate 0 for all
  //!
  void disableChannel(const size_t& channel);

  //!
  //! \brief request feedback (position and current) to a specific channel
  //! \param channel Motorchannel the feedback should be provided for
  //!
  void getControllerFeedback(const size_t& channel);

  /*!
   * \brief request the settings of the position controller for a specific channel
   * \param channel Motor to request the settings for
   */
  void getPositionSettings(const size_t& channel);

  /*!
   * \brief activate a new set of position controller settings for a specific channel
   * \param channel Motor the new position controller settings will be applied to
   * \param position_settings new settings of the position controller
   */
  void setPositionSettings(const size_t& channel,const S5FHPositionSettings& position_settings);

  /*!
   * \brief request the settings of the current controller for a specific channel
   * \param channel Motor to request the settings for
   */
  void getCurrentSettings(const size_t& channel);

  /*!
   * \brief activate a new set of current controller settings for a specific channel
   * \param channel Motor the new current controller settings will be applied to
   * \param current_settings new settings of the current controller
   */
  void setCurrentSettings(const size_t& channel,const S5FHCurrentSettings& current_settings);

  /*!
   * \brief read out the mutipliers for the encoders
   */
  void getEncoderValues();

  //TODO: EncoderValues Type
  //void setEncoderValues

  /*!
   * \brief request a transmission of formware information
   */
  void getFirmwareInfo();


  void receivedPacketCallback(const S5FHSerialPacket& packet, unsigned int packet_count);


 //! definition of boost function callback for received packages
 //typedef boost::function<void (const S5FHSerialPacket& packet, unsigned int packet_count)> ReceivedPacketCallback;

private:

  // Data Structures for holding configurations and feedback of the Controller

  //! vector of current controller parameters for each finger
  std::vector<S5FHCurrentSettings> m_current_settings;

  //! vector of position controller parameters for each finger
  std::vector<S5FHPositionSettings> m_position_settings;

  //! ControllerFeedback indicates current position and current per finger
  std::vector<S5FHControllerFeedback> m_controller_feedback;

  //! Currently active controllerstate on the HW Controller (indicates if PWM active etc.)
  S5FHControllerState m_controller_state;


  // Hardware control

  //! Serial interface for transmission and reveibing of data packets
  S5FHSerialInterface * m_serial_interface;




};

}

#endif
