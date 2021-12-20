# Schunk SVH Library

TODO:

## Installation

Make sure that `cmake` finds IC-related exports with
setting an environment variable in the shell for cmake (adjust to your setup):

```bash
export CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH:/home/scherzin/src/robot_folders/checkout/schunk_svh_ros1/ic_workspace/export
```

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
