cmake_minimum_required(VERSION 3.1)
project(CommandFramework C CXX)

find_package(THREADS REQUIRED)

include_directories(include)
file(GLOB FRAMEWORK_SRC src/*.c src/*.cpp)

add_library(${PROJECT_NAME} STATIC ${FRAMEWORK_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})

# Expose public includes to other subprojects through cache variable.
set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)