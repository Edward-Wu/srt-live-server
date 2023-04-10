# set minimum cmake version
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# project name and language
project(srt-live-server VERSION 1.4.8 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SLSCORE_LIST slscore/*.cpp)
file(GLOB SLSCORE_HEAD_LIST slscore/*.hpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SRT IMPORTED_TARGET srt)
include_directories(${SRT_INCLUDE_DIRS})
link_directories(${SRT_LIBRARY_DIRS})

include_directories(slscore)
add_executable(sls srt-live-server.cpp ${SLSCORE_LIST})
add_executable(slc srt-live-client.cpp ${SLSCORE_LIST})

set_target_properties(sls PROPERTIES
    COMPILE_FLAGS "-pthread"
    LINK_FLAGS "-pthread")

set_target_properties(slc PROPERTIES
    COMPILE_FLAGS "-pthread"
    LINK_FLAGS "-pthread")

target_link_libraries(sls PkgConfig::SRT)
target_link_libraries(slc PkgConfig::SRT)