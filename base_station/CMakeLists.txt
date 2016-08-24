cmake_minimum_required(VERSION 3.1)
project(BaseStation C CXX)

# TODO: GUI dependencies?

file(GLOB BASE_STATION_SRC src/*.c src/*.cpp)

add_definitions(${CommandFramework_DEFINITIONS})
include_directories(${CommandFramework_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${ROBOT_SRC})
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} CommandFramework)