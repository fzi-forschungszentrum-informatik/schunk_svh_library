# Schunk SVH Library

TODO:

## Installation
Inside the `ic_workspace` folder, call
```bash
mkdir build && cd build
cmake ..
make -j6
make install
```

Calling `install` is actually important to have all CMake targets in the right place.
Note that ROS1 didn't require this and allowed users to work primariliy with the `devel`.

## Running tests manually

We currently use the `Boost` test framework.
You can call the tests manually on the command line with

```bash
make CTEST_OUTPUT_ON_FAILURE=1 test
```



