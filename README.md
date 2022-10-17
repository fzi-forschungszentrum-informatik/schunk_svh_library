![build badge](https://github.com/fzi-forschungszentrum-informatik/schunk_svh_library/actions/workflows/industrial_ci_melodic_action.yml/badge.svg)
![build badge](https://github.com/fzi-forschungszentrum-informatik/schunk_svh_library/actions/workflows/industrial_ci_noetic_action.yml/badge.svg)
![build badge](https://github.com/fzi-forschungszentrum-informatik/schunk_svh_library/actions/workflows/industrial_ci_foxy_action.yml/badge.svg)
![build badge](https://github.com/fzi-forschungszentrum-informatik/schunk_svh_library/actions/workflows/industrial_ci_humble_action.yml/badge.svg)
[![License](https://img.shields.io/badge/License-GPLv3-orange)](https://opensource.org/licenses/gpl-license)

# Schunk SVH Library

This is a standalone C++ library to control the Schunk SVH 5-finger hand.
It provides a hardware abstraction for driver development.
Users can develop custom wrappers on top in their preferred robotics framework.

## Installation

---

**Note:** Switch to [these instructions](https://github.com/fzi-forschungszentrum-informatik/schunk_svh_ros_driver) when working in a ROS1/ROS2 context.

---

Start an *out of source build* in the parent folder of this package with
```bash
mkdir build && cd build
cmake ../driver_svh/ -DCMAKE_INSTALL_PREFIX=../install
cmake --build .
make install
```
You may also provide another install folder via `-DCMAKE_INSTALL_PREFIX=`.

## Connection to the SVH

This driver library uses a *serial* interface to communicate with the SVH.
We assume that you use a suitable *usb-serial dongle* and that the dongle's driver provides a `/dev/ttyUSB0` file for communication with the hardware.
Recent Ubuntu operating systems should support that out-of-the-box.
However, it will be necessary to add your user to the `dialout` group to speak with that device file without sudo privileges.
Call
```bash
usermod -a -G dialout $USER
```
and restart your system.
After that, you are good to go.

## Running tests manually

We currently use the `Boost` test framework.
You can run the tests manually on the command line with

```bash
make CTEST_OUTPUT_ON_FAILURE=1 test
```
