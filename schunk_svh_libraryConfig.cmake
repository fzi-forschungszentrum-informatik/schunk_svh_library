include(CMakeFindDependencyMacro)
#find_dependency(some-other-package)

include("${CMAKE_CURRENT_LIST_DIR}/schunk_svh_libraryTargets.cmake")

# This is for catkin compatibility. Better use target_link_libraries(<my_target> schunk_svh_library::svh-library)
set(schunk_svh_library_LIBRARIES schunk_svh_library::svh-library)
get_target_property(schunk_svh_library_INCLUDE_DIRS schunk_svh_library::schunk_svh_library INTERFACE_INCLUDE_DIRECTORIES)
