cmake_minimum_required (VERSION 3.14 FATAL_ERROR)

if(${CMAKE_VERSION} VERSION_LESS 3.16)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.17)
endif()

# The version number.
set (Password_Maker_VERSION_MAJOR 0)
set (Password_Maker_VERSION_MINOR 0)
set (Password_Maker_VERSION_PATCH 3)
set (Password_Maker_VERSION "0.0.3")
string(TIMESTAMP Password_Maker_COMPILE_DATE "%Y-%m-%d")

project("password_maker" LANGUAGES CXX VERSION ${Password_Maker_VERSION})

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/maker/inc/version.h.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/maker/inc/version.h"
)

add_subdirectory("maker")