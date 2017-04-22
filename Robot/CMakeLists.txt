cmake_minimum_required(VERSION 3.1)
project(LAME)

# external dependencies
find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)

# C++11 required
set (CMAKE_CXX_STANDARD 11)

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(SEND_ERROR "In-source builds are not allowed.")
endif ()

# Remove 'lib' prefix for shared libraries on Windows
if (WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif ()

# JoystickLibrary.lib should be located in the Robot/ folder
link_directories(.)

# include directories
include_directories(${CMAKE_SOURCE_DIR}/../JoystickLibrary/cpp/include include)

file(GLOB ROBOT_SRC src/*.cpp src/*.c include/*.hpp include/*.h)

add_executable(${PROJECT_NAME} ${ROBOT_SRC})
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${OpenCV_LIBS} JoystickLibrary)
