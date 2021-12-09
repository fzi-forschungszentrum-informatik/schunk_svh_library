cmake_minimum_required(VERSION 3.10)

project(schunk_svh_library VERSION 1.0 DESCRIPTION "C++ driver library for the Schunk SVH")

# CMake will automatically update this library's version in the
# SVHVersionConfig.h include.
configure_file(include/schunk_svh_library/VersionConfig.h.in SVHVersionConfig.h)

# We require this C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# System dependencies
find_package(icl_comm)
find_package(icl_core)

# --------------------------------------------------------------------------------
# User library
# --------------------------------------------------------------------------------
add_library(schunk_svh SHARED
        src/Logging.cpp
        src/control/SVHController.cpp
        src/control/SVHFeedbackPollingThread.cpp
        src/control/SVHFingerManager.cpp
        )

target_include_directories(schunk_svh PUBLIC
        "${PROJECT_BINARY_DIR}"
        "${PROJECT_SOURCE_DIR}/include"
        )

target_link_libraries(schunk_svh PUBLIC
        svh_serial
        )

# --------------------------------------------------------------------------------
# Internal libraries
# --------------------------------------------------------------------------------
add_library(svh_serial SHARED
        src/serial/ByteOrderConversion.cpp
        src/serial/Serial.cpp
        src/serial/SerialFlags.cpp
        src/serial/SVHReceiveThread.cpp
        src/serial/SVHSerialInterface.cpp
        src/serial/SVHSerialPacket.cpp
        )

target_include_directories(svh_serial PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        "${icl_comm_INCLUDE_DIRS}"
        "${icl_core_INCLUDE_DIRS}"
        )

target_link_libraries(svh_serial PUBLIC
        ${icl_comm_LIBRARIES}
        ${icl_core_LIBRARIES}
        )

# Leftovers by icl_comm and icl_core
target_compile_definitions(svh_serial PUBLIC
        "-D_SYSTEM_LINUX_"
        "-D_SYSTEM_POSIX_"
        )

