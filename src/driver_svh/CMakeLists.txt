# this is for emacs file handling -*- mode: cmake; indent-tabs-mode: nil -*-
ICMAKER_SET("driver_svh" IDE_FOLDER ${DRIVER_SVH_IDE_FOLDER})

ICMAKER_ADD_SOURCES(
  Logging.cpp
  SVHController.cpp
  SVHFeedbackPollingThread.cpp
  SVHFingerManager.cpp
  SVHReceiveThread.cpp
  SVHSerialInterface.cpp
  SVHSerialPacket.cpp
)

ICMAKER_ADD_HEADERS(
  ImportExport.h
  Logging.h
  SVHController.h
  SVHSerialPacket.h
  SVHPositionSettings.h
  SVHCurrentSettings.h
  SVHFirmwareInfo.h
  SVHControllerFeedback.h
  SVHControllerState.h
  SVHControlCommand.h
  SVHFeedbackPollingThread.h
  SVHFingerManager.h
  SVHReceiveThread.h
  SVHSerialInterface.h
  SVHEncoderSettings.h
)

ICMAKER_LOCAL_CPPDEFINES(-DDRIVER_SVH_EXPORT_SYMBOLS)
ICMAKER_GLOBAL_CPPDEFINES(-D_IC_BUILDER_DRIVER_SVH_)
ICMAKER_INCLUDE_DIRECTORIES(${DRIVER_SVH_INCLUDE_DIRS})

ICMAKER_INTERNAL_DEPENDENCIES(
  icl_core
  icl_core_logging
  icl_core_config
  icl_core_thread
  icl_comm
  icl_comm_serial

  icl_comm_websocket # OPTIONAL
)

ICMAKER_BUILD_LIBRARY()
ICMAKER_INSTALL_HEADERS(${icmaker_target})
