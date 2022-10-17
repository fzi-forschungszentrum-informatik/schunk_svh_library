cmake_minimum_required(VERSION 3.10)

project(schunk_svh_library VERSION 1.0 DESCRIPTION "C++ driver library for the Schunk SVH")

# CMake will automatically update this library's version in the
# SVHVersionConfig.h include.
configure_file(include/schunk_svh_library/VersionConfig.h.in SVHVersionConfig.h)

# We require this C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# System dependencies
find_package(Threads REQUIRED)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

# --------------------------------------------------------------------------------
# Make sure that library relocation works for both build and install.
# See here: https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling
# --------------------------------------------------------------------------------

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# The RPATH to use when installing, but only if it's not a system directory
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif("${isSystemDir}" STREQUAL "-1")

# --------------------------------------------------------------------------------
# User library
# --------------------------------------------------------------------------------
add_library(svh-library SHARED
        src/control/SVHController.cpp
        src/control/SVHFingerManager.cpp
        )

# Provide an alias target for our users' call to target_link_libraries()
add_library(Schunk::svh-library ALIAS svh-library)

target_include_directories(svh-library PUBLIC
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )

target_link_libraries(svh-library PUBLIC
        svh-serial
        )


# --------------------------------------------------------------------------------
# Internal libraries
# --------------------------------------------------------------------------------
add_library(svh-serial SHARED
        src/LogHandler.cpp
        src/serial/ByteOrderConversion.cpp
        src/serial/Serial.cpp
        src/serial/SerialFlags.cpp
        src/serial/SVHReceiveThread.cpp
        src/serial/SVHSerialInterface.cpp
        src/serial/SVHSerialPacket.cpp
        )

add_library(Schunk::svh-serial ALIAS svh-serial)

target_include_directories(svh-serial PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )

target_link_libraries(svh-serial PUBLIC
        )
target_compile_definitions(svh-serial PUBLIC
        -D_SYSTEM_LINUX_
        -D_SYSTEM_POSIX_
        )
if(THREADS_HAVE_PTHREAD_ARG)
  target_compile_options(svh-serial PUBLIC "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(svh-serial PUBLIC "${CMAKE_THREAD_LIBS_INIT}")
endif()


# --------------------------------------------------------------------------------


# --------------------------------------------------------------------------------
# Tests
# --------------------------------------------------------------------------------
add_executable(test_driver_svh
        test/driver_svh/MainTest.cpp
        test/driver_svh/ByteOrderConversionTest.cpp
        test/driver_svh/SVHDriverTest.cpp
        )
target_include_directories(test_driver_svh PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        )
target_link_libraries(test_driver_svh
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        svh-library
        )
target_compile_definitions(test_driver_svh PUBLIC
        -D_SYSTEM_LINUX_
        -D_SYSTEM_POSIX_
        -DBOOST_TEST_DYN_LINK
        )
add_test(NAME test_driver_svh COMMAND test_driver_svh)

# --------------------------------------------------------------------------------

add_executable(test_svh_send_packet
        test/serial_interface/SVHSendPacketTest.cpp
        )
target_include_directories(test_svh_send_packet PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        )
target_link_libraries(test_svh_send_packet
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        svh-serial
        )
add_test(NAME test_svh_send_packet COMMAND test_svh_send_packet)

# --------------------------------------------------------------------------------

#add_executable(test_svh_receive
#        test/serial_interface/SVHReceiveTest.cpp
#        )
#target_include_directories(test_svh_receive PUBLIC
#        ${PROJECT_SOURCE_DIR}/include
#        )
#target_link_libraries(test_svh_receive
#        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
#        svh-serial
#        )
#add_test(NAME test_svh_receive COMMAND test_svh_receive)
#set_tests_properties(test_svh_receive PROPERTIES TIMEOUT 10)

# --------------------------------------------------------------------------------

add_executable(test_svh_send_feedback_packet
        test/serial_interface/SVHSendFeedbackPacketTest.cpp
        )
target_include_directories(test_svh_send_feedback_packet PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        )
target_link_libraries(test_svh_send_feedback_packet
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        svh-serial
        )
add_test(NAME test_svh_send_feedback_packet COMMAND test_svh_send_feedback_packet)

# --------------------------------------------------------------------------------

add_executable(test_svh_controller_init_send
        test/serial_interface/SVHControllerInitSendTest.cpp
        )
target_include_directories(test_svh_controller_init_send PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        )
target_link_libraries(test_svh_controller_init_send
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        svh-library
        )
add_test(NAME test_svh_controller_init_send COMMAND test_svh_controller_init_send)

# --------------------------------------------------------------------------------

add_executable(test_svh_finger_manager
        test/serial_interface/SVHFingerManagerTest.cpp
        )
target_include_directories(test_svh_finger_manager PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        )
target_link_libraries(test_svh_finger_manager
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        svh-library
        )
add_test(NAME test_svh_finger_manager COMMAND test_svh_finger_manager)

# --------------------------------------------------------------------------------

enable_testing()

# --------------------------------------------------------------------------------
# Export and install
# --------------------------------------------------------------------------------
install(TARGETS
        svh-library
        svh-serial
        EXPORT schunk_svh_library_targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
        )

# Create and install a file with all exported targets
install(EXPORT schunk_svh_library_targets
        DESTINATION lib/cmake/schunk_svh_library
        FILE schunk_svh_libraryTargets.cmake
        NAMESPACE Schunk::
        )

# Install the Config file so that find_package() works with this package
install(FILES schunk_svh_libraryConfig.cmake
        DESTINATION lib/cmake/schunk_svh_library
        )

# Install the include directories
install(DIRECTORY include/schunk_svh_library
  DESTINATION include
  FILES_MATCHING PATTERN "*.h")

#### Begin import ####
# Imported from ros-industrial/ros_industrial_cmake_boilerplate
# https://github.com/ros-industrial/ros_industrial_cmake_boilerplate/blob/master/ros_industrial_cmake_boilerplate/cmake/cmake_tools.cmake
# Copyright (C) 2018 by George Cave - gcave@stablecoder.ca
# Copyright (c) 2020, Southwest Research Institute
# Licensed under Apache-2.0 license

# Allows Colcon to find non-Ament packages when using workspace underlays
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/share/ament_index/resource_index/packages/${PROJECT_NAME} "")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/share/ament_index/resource_index/packages/${PROJECT_NAME} DESTINATION share/ament_index/resource_index/packages)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/share/${PROJECT_NAME}/hook/ament_prefix_path.dsv "prepend-non-duplicate;AMENT_PREFIX_PATH;")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/share/${PROJECT_NAME}/hook/ament_prefix_path.dsv DESTINATION share/${PROJECT_NAME}/hook)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/share/${PROJECT_NAME}/hook/ros_package_path.dsv "prepend-non-duplicate;ROS_PACKAGE_PATH;")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/share/${PROJECT_NAME}/hook/ros_package_path.dsv DESTINATION share/${PROJECT_NAME}/hook)
#### End import ####
