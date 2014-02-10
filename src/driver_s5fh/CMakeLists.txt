# this is for emacs file handling -*- mode: cmake; indent-tabs-mode: nil -*-
ICMAKER_SET("driver_s5fh" IDE_FOLDER ${DRIVER_S5FH_IDE_FOLDER})

ICMAKER_ADD_SOURCES(
  Logging.cpp
  S5FHController.cpp
  S5FHFingerManager.cpp
  S5FHReceiveThread.cpp
  S5FHSerialInterface.cpp
  S5FHSerialPacket.cpp
)

ICMAKER_ADD_HEADERS(
  ImportExport.h
  Logging.h
  S5FHController.h
  S5FHSerialPacket.h
  S5FHPositionSettings.h
  S5FHCurrentSettings.h
  S5FHFirmwareInfo.h
  S5FHControllerFeedback.h
  S5FHControllerState.h
  S5FHControlCommand.h
  S5FHFingerManager.h
  S5FHReceiveThread.h
  S5FHSerialInterface.h
)

ICMAKER_LOCAL_CPPDEFINES(-DDRIVER_S5FH_EXPORT_SYMBOLS)
ICMAKER_GLOBAL_CPPDEFINES(-D_IC_BUILDER_DRIVER_S5FH_)
ICMAKER_INCLUDE_DIRECTORIES(${DRIVER_S5FH_INCLUDE_DIRS})

ICMAKER_INTERNAL_DEPENDENCIES(
  icl_core
  icl_core_logging
  icl_core_config
  icl_core_thread
  icl_comm
  icl_comm_serial
)

ICMAKER_BUILD_LIBRARY()
ICMAKER_INSTALL_HEADERS(${icmaker_target})
