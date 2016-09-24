cmake_minimum_required(VERSION 3.1)
project(Robot C CXX)

include_directories(include)
file(GLOB ROBOT_SRC src/*.c src/*.cpp include/*.hpp)

add_definitions(${CommandFramework_DEFINITIONS})
include_directories(${CommandFramework_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${ROBOT_SRC})
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} CommandFramework JoystickLibrary)