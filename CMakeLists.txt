cmake_minimum_required(VERSION 3.13)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")

project(illarion
        VERSION 0.9.16
        DESCRIPTION "Server for the Illarion MORPG"
        HOMEPAGE_URL "https://github.com/Illarion-eV/Illarion-Server")

if( CMAKE_BUILD_TYPE STREQUAL "" )
  set( CMAKE_BUILD_TYPE "Debug" )
endif()

find_package( Boost REQUIRED )
add_subdirectory( extern EXCLUDE_FROM_ALL )

enable_testing()
include(GoogleTest)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options( -Wall -Wformat-security -Wunused-result -Werror -Wno-deprecated -pedantic-errors )

add_subdirectory( src )
add_subdirectory( test )
