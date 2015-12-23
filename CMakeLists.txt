cmake_minimum_required (VERSION 2.8.11)
project (IPCommand)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules")

find_package(GLIB COMPONENTS gio)

add_definitions(-DDEBUG)

include_directories(SYSTEM include)
add_subdirectory(src/ipcomm)
add_subdirectory(src/tem2)