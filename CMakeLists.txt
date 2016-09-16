# Adapted from http://stackoverflow.com/questions/16398937/cmake-and-finding-other-projects-and-their-dependencies
# to build with no gui - run cmake with -DNOGUI:BOOL=true
cmake_minimum_required(VERSION 3.1)

set (CMAKE_CXX_STANDARD 11)

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(SEND_ERROR "In-source builds are not allowed.")
endif ()

# Remove 'lib' prefix for shared libraries on Windows
if (WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif ()

# configure subprojects
add_subdirectory(framework)
add_subdirectory(robot)
