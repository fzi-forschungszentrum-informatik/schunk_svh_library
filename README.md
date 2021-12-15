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

## Running tests manually

We currently use the `Boost` test framework.
You can run the tests manually on the command line with

```bash
make CTEST_OUTPUT_ON_FAILURE=1 test
```
