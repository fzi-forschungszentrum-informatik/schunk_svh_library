# this is for emacs file handling -*- mode: cmake; indent-tabs-mode: nil -*-
ICMAKER_SET("ts_S5FHSendPacket" IDE_FOLDER ${DRIVER_S5FH_IDE_FOLDER})

ICMAKER_ADD_SOURCES(
  S5FHSendPacketTest.cpp
  )

ICMAKER_INCLUDE_DIRECTORIES( ${DRIVER_S5FH_INCLUDE_DIRS} )

ICMAKER_EXTERNAL_DEPENDENCIES(
  )

ICMAKER_INTERNAL_DEPENDENCIES(
  driver_s5fh
  icl_core
  icl_core_thread
  icl_comm
  )

ICMAKER_BUILD_TEST()

############################################################################

ICMAKER_SET("ts_S5FHReceive" IDE_FOLDER ${DRIVER_S5FH_IDE_FOLDER})

ICMAKER_ADD_SOURCES(
  S5FHReceiveTest.cpp
  )

ICMAKER_INCLUDE_DIRECTORIES( ${DRIVER_S5FH_INCLUDE_DIRS} )

ICMAKER_EXTERNAL_DEPENDENCIES(
  )

ICMAKER_INTERNAL_DEPENDENCIES(
  driver_s5fh
  icl_core
  icl_core_thread
  icl_comm
  )

ICMAKER_BUILD_TEST()

#############################################################################

ICMAKER_SET("ts_S5FHSendFeedbackPacket" IDE_FOLDER ${DRIVER_S5FH_IDE_FOLDER})

ICMAKER_ADD_SOURCES(
  S5FHSendFeedbackPacketTest.cpp
  )

ICMAKER_INCLUDE_DIRECTORIES( ${DRIVER_S5FH_INCLUDE_DIRS} )

ICMAKER_EXTERNAL_DEPENDENCIES(
  )

ICMAKER_INTERNAL_DEPENDENCIES(
  driver_s5fh
  icl_core
  icl_core_thread
  icl_comm
  )

ICMAKER_BUILD_TEST()

##############################################################################

ICMAKER_SET("ts_S5FHReceiveFeedbackPacket" IDE_FOLDER ${DRIVER_S5FH_IDE_FOLDER})

ICMAKER_ADD_SOURCES(
  S5FHReceiveFeedbackPacketTest.cpp
  )

ICMAKER_INCLUDE_DIRECTORIES( ${DRIVER_S5FH_INCLUDE_DIRS} )

ICMAKER_EXTERNAL_DEPENDENCIES(
  )

ICMAKER_INTERNAL_DEPENDENCIES(
  driver_s5fh
  icl_core
  icl_core_thread
  icl_comm
  )

ICMAKER_BUILD_TEST()
