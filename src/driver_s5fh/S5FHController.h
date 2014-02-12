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
#include <driver_s5fh/S5FHEncoderSettings.h>

namespace driver_s5fh {

//! Channel indicates which motor to use in command calls. WARNING: DO NOT CHANGE THE ORDER OF THESE
enum{
  eS5FH_ALL = -1,
  eS5FH_THUMB_FLEXION = 0,
  eS5FH_THUMB_OPPOSITION, // wrist
  eS5FH_INDEX_FINGER_DISTAL,
  eS5FH_INDEX_FINGER_PROXIMAL,
  eS5FH_MIDDLE_FINGER_DISTAL,
  eS5FH_MIDDLE_FINGER_PROXIMAL,
  eS5FH_RING_FINGER,
  eS5FH_PINKY,
  eS5FH_FINGER_SPREAD,
  eS5FH_DIMENSION
} typedef S5FHCHANNEL;


/*! This class controls the the SCHUNK five finger hand.
 */
class DRIVER_S5FH_IMPORT_EXPORT S5FHController
{
public:
  /*! Constructs a controller class for the SCHUNK five finger hand.
   */
  S5FHController();

  /*! SCHUNK five finger hand destructor
   *  Disable the serial device and shut down hand as far
   *  as possible
   */
  ~S5FHController();

  //!
  //! \brief open serial device connection
  //! \param dev_name
  //! \return
  //!
  bool connect(const std::string &dev_name);

  //!
  //! \brief disconnect serial device
  //!
  void disconnect();

  /*!
   * \brief setting new position controller target for finger index
   * \param channel Motorchanel to set the target for
   * \param position Target position given in encoder Ticks
   */
  void setControllerTarget(const S5FHCHANNEL& channel, const u_int32_t& position);


  // Access functions
  //!
  //! \brief Enable one or all motor channels
  //! \param channel Motor to activate 0 for all
  //!
  void enableChannel(const S5FHCHANNEL& channel);

  //!
  //! \brief Disable one or all motor channels
  //! \param channel Motor to deactivate 0 for all
  //!
  void disableChannel(const S5FHCHANNEL& channel);

  //!
  //! \brief request feedback (position and current) to a specific channel
  //! \param channel Motorchannel the feedback should be provided for
  //!
  void requestControllerFeedback(const S5FHCHANNEL& channel);

  /*!
   * \brief request the settings of the position controller for a specific channel
   * \param channel Motor to request the settings for
   */
  void requestPositionSettings(const S5FHCHANNEL& channel);

  /*!
   * \brief activate a new set of position controller settings for a specific channel
   * \param channel Motor the new position controller settings will be applied to
   * \param position_settings new settings of the position controller
   */
  void setPositionSettings(const S5FHCHANNEL& channel,const S5FHPositionSettings& position_settings);

  /*!
   * \brief request the settings of the current controller for a specific channel
   * \param channel Motor to request the settings for
   */
  void requestCurrentSettings(const S5FHCHANNEL& channel);

  /*!
   * \brief activate a new set of current controller settings for a specific channel
   * \param channel Motor the new current controller settings will be applied to
   * \param current_settings new settings of the current controller
   */
  void setCurrentSettings(const S5FHCHANNEL& channel,const S5FHCurrentSettings& current_settings);

  /*!
   * \brief read out the mutipliers for the encoders from the hardware
   */
  void requestEncoderValues();

  /*!
   * \brief sends a new set of encodervalues to the hardware
   * \param encoder_settings to set (prescalers)
   */
  void setEncoderValues(const S5FHEncoderSettings& encoder_settings);


  /*!
   * \brief request a transmission of formware information
   */
  void requestFirmwareInfo();

  /*!
   * \brief callback function for interpretation of packages
   * \param packet SerialPacket containing the raw data, integrity should have been checked by SerialInterface
   * \param packet_count count of received packets
   */
  void receivedPacketCallback(const S5FHSerialPacket& packet, unsigned int packet_count);

  /*!
   * \brief request the latest stored controllerfeedback (current, position) from the controller.
   * \param channel Motor to get the latest feedback to
   * \param ControllerFeedback (current, encoder position) of the specified channel
   * \return true if the feedback could be read, false otherwise
   *
   * Controllerfeedback (crurrent,channel) is stored in the controller once it is send by the hardware.
   * This is the case once a controlCommand (TODO: MeCoVis is this correct?) has been send or the feedback has
   * specifically been requested by using the getControllerFeedback() function
   *
   */
   bool getControllerFeedback(const S5FHCHANNEL &channel,S5FHControllerFeedback& controller_feedback);

   /*!
    * \brief request the latest stored positionsettings from the controller
    * \param channel Motor to get the positionsettings for
    * \param position_settings position settings to be returned
    * \return true if the request was succesfull false otherwise
    */
   bool getPositionSettings(const S5FHCHANNEL &channel,S5FHPositionSettings& position_settings);

   /*!
    * \brief request the latest stored currentsettings from the controller
    * \param channel Motor to get the currentsettings for
    * \param position_settings current settings to be returned
    * \return true if the request was succesfull false otherwise
    */
   bool getCurrentSettings(const S5FHCHANNEL &channel,S5FHPositionSettings& position_settings);

   /*!
    * \brief Check if a channel was enabled
    * \param channel to check
    * \return True if an enable has been send to the hardware
    */
   bool isEnabled(const S5FHCHANNEL &channel);

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

  //! Currently active encoder settings
  S5FHEncoderSettings m_encoder_settings;

  //! Latest firmware info
  S5FHFirmwareInfo m_firmware_info;

  // Hardware control

  //! Serial interface for transmission and reveibing of data packets
  S5FHSerialInterface * m_serial_interface;

  //! Bitmask to tell which fingers are enabled
  u_int16_t m_enable_mask;




};

}

#endif
