include(CMakeFindDependencyMacro)
#find_dependency(some-other-package)

include("${CMAKE_CURRENT_LIST_DIR}/schunk_svh_libraryTargets.cmake")

# This is for catkin compatibility. Better use target_link_libraries(<my_target> Schunk::svh-library)
set(schunk_svh_library_LIBRARIES Schunk::svh-library)
get_target_property(schunk_svh_library_INCLUDE_DIRS Schunk::svh-library INTERFACE_INCLUDE_DIRECTORIES)
