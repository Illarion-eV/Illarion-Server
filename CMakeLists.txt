cmake_minimum_required(VERSION 3.21)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")

find_package( Git )

if( GIT_FOUND )
  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --tags --long
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE illarion_VERSION_GIT
    RESULT_VARIABLE version_error
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if( version_error )
    message( WARNING "Failed to get version information from git: ${version_error}" )
    set( illarion_VERSION_GIT "42.42.42-gdeadbee" )
    set( illarion_VERSION_DESC "unversioned" )
  else()
    set( illarion_VERSION_DESC ${illarion_VERSION_GIT} )
  endif()
else()
  message( WARNING "Git is not available. Can't get version information." )
  set( illarion_VERSION_GIT "42.42.42-gdeadbee" )
  set( illarion_VERSION_DESC "unknown" )
endif()

string( REGEX MATCH "[0-9]+(\\.[0-9]+)+" illarion_VERSION "${illarion_VERSION_GIT}" )
message( STATUS "Illarion Server version: ${illarion_VERSION} (${illarion_VERSION_DESC})" )

cmake_policy( SET CMP0048 NEW )

project(illarion
        VERSION ${illarion_VERSION}
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
add_compile_definitions(BOOST_BIND_GLOBAL_PLACEHOLDERS)

add_subdirectory( src )
add_subdirectory( test )
