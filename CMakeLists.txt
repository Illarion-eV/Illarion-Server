cmake_minimum_required(VERSION 3.13)

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/" )

project(illarion
        VERSION 0.9.15
        DESCRIPTION "Server for the Illarion MORPG"
        HOMEPAGE_URL "https://github.com/Illarion-eV/Illarion-Server")

enable_testing()
include(GoogleTest)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options( -Wall -Wformat-security -Wunused-result -Werror -Wno-deprecated -pedantic-errors )

add_subdirectory(src)
add_subdirectory(test)