cmake_minimum_required(VERSION 3.10)

project(schunk_svh_library VERSION 1.0 DESCRIPTION "C++ driver library for the Schunk SVH")

# CMake will automatically update this library's version in the
# SVHVersionConfig.h include.
configure_file(include/schunk_svh_library/VersionConfig.h.in SVHVersionConfig.h)

# We require this C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# System dependencies
find_package(icl_core)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)
pkg_check_modules(ZMQ REQUIRED libzmq)
find_package(Boost COMPONENTS thread REQUIRED)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

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
        svh_websocket
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
        src/Logging.cpp
        )

target_include_directories(svh_serial PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        "${icl_core_INCLUDE_DIRS}"
        )

target_link_libraries(svh_serial PUBLIC
        ${icl_core_LIBRARIES}
        )

# Leftovers by icl_comm and icl_core
target_compile_definitions(svh_serial PUBLIC
        "-D_SYSTEM_LINUX_"
        "-D_SYSTEM_POSIX_"
        )

# --------------------------------------------------------------------------------

add_library(svh_websocket SHARED
        src/websocket/ZMQClient.cpp
        src/websocket/RobotState.cpp
        src/websocket/LWA4PState.cpp
        src/websocket/SVHState.cpp
        src/websocket/WsBroadcaster.cpp
        )

target_include_directories(svh_websocket PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        "${JSONCPP_INCLUDE_DIRS}"
        "${icl_core_INCLUDE_DIRS}"
        )

target_link_libraries(svh_websocket PUBLIC
        ${icl_core_LIBRARIES}
        "${JSONCPP_LIBRARIES}"
        "${ZMQ_LIBRARIES}"
        Boost::thread
        )

# Leftovers by icl_core
target_compile_definitions(svh_websocket PUBLIC
        "-D_SYSTEM_LINUX_"
        "-D_SYSTEM_POSIX_"
        )

# --------------------------------------------------------------------------------
# Tests
# --------------------------------------------------------------------------------
add_executable(test_driver_svh
        test/driver_svh/ByteOrderConversionTest.cpp
        test/driver_svh/MainTest.cpp
        test/driver_svh/SVHDriverTest.cpp
        )
target_include_directories(test_driver_svh PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        "${icl_core_INCLUDE_DIRS}"
        )
target_link_libraries(test_driver_svh
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        ${icl_core_LIBRARIES}
        schunk_svh
        )
target_compile_definitions(test_driver_svh PUBLIC
        "-D_SYSTEM_LINUX_"
        "-D_SYSTEM_POSIX_"
        )
add_test(NAME test_driver_svh COMMAND test_driver_svh)

# --------------------------------------------------------------------------------

add_executable(test_svh_send_packet
        test/serial_interface/SVHSendPacketTest.cpp
        )
target_include_directories(test_svh_send_packet PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        "${icl_core_INCLUDE_DIRS}"
        )
target_link_libraries(test_svh_send_packet
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        svh_serial
        )
add_test(NAME test_svh_send_packet COMMAND test_svh_send_packet)

# --------------------------------------------------------------------------------

add_executable(test_svh_receive
        test/serial_interface/SVHReceiveTest.cpp
        )
target_include_directories(test_svh_receive PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        "${icl_core_INCLUDE_DIRS}"
        )
target_link_libraries(test_svh_receive
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        svh_serial
        )
add_test(NAME test_svh_receive COMMAND test_svh_receive)
set_tests_properties(test_svh_receive PROPERTIES TIMEOUT 10) 

# --------------------------------------------------------------------------------

add_executable(test_svh_send_feedback_packet
        test/serial_interface/SVHSendFeedbackPacketTest.cpp
        )
target_include_directories(test_svh_send_feedback_packet PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        "${icl_core_INCLUDE_DIRS}"
        )
target_link_libraries(test_svh_send_feedback_packet
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        svh_serial
        )
add_test(NAME test_svh_send_feedback_packet COMMAND test_svh_send_feedback_packet)

# --------------------------------------------------------------------------------

add_executable(test_svh_controller_init_send
        test/serial_interface/SVHControllerInitSendTest.cpp
        )
target_include_directories(test_svh_controller_init_send PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        "${icl_core_INCLUDE_DIRS}"
        )
target_link_libraries(test_svh_controller_init_send
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        schunk_svh
        )
add_test(NAME test_svh_controller_init_send COMMAND test_svh_controller_init_send)

# --------------------------------------------------------------------------------

add_executable(test_svh_finger_manager
        test/serial_interface/SVHFingerManagerTest.cpp
        )
target_include_directories(test_svh_finger_manager PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        "${icl_core_INCLUDE_DIRS}"
        )
target_link_libraries(test_svh_finger_manager
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        schunk_svh
        )
add_test(NAME test_svh_finger_manager COMMAND test_svh_finger_manager)

# --------------------------------------------------------------------------------
enable_testing()
