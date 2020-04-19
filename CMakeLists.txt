cmake_minimum_required (VERSION 3.14 FATAL_ERROR)

if(${CMAKE_VERSION} VERSION_LESS 3.16)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.17)
endif()

set(VERSION_REGEX "#define PASSWORD_MAKER_VERSION[ \t]+\"(.+)\"")

# Read in the line containing the version
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/maker/inc/version.h"
     VERSION_STRING REGEX ${VERSION_REGEX})

# Pick out just the version
string(REGEX REPLACE ${VERSION_REGEX} "\\1" VERSION_STRING "${VERSION_STRING}")

project("password_maker" LANGUAGES CXX VERSION ${VERSION_STRING})

add_subdirectory("maker")