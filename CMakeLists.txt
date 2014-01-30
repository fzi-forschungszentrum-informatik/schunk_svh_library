# this is for emacs file handling -*- mode: cmake; indent-tabs-mode: nil -*-

# To be used by other modules
SET(DRIVER_S5FH_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src" "${CMAKE_CURRENT_BINARY_DIR}/src" CACHE INTERNAL "")
SET(DRIVER_S5FH_IDL_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src" CACHE INTERNAL "")
SET(DRIVER_S5FH_IDE_FOLDER "driver_s5fh")

ICMAKER_REGISTER_PACKAGE(driver_s5fh)

ADD_SUBDIRECTORY (src/driver_s5fh)

ICMAKER_CONFIGURE_PACKAGE()
