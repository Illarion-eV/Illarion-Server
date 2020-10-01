cmake_minimum_required(VERSION 3.13)

project(illarion
        VERSION 0.9.15
        DESCRIPTION "Server for the Illarion MORPG"
        HOMEPAGE_URL "https://github.com/Illarion-eV/Illarion-Server")

enable_testing()
include(GoogleTest)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(WARNINGS -Wall -Wformat-security -Wunused-result -Werror -Wno-deprecated -pedantic-errors)

add_subdirectory(src)
add_subdirectory(test)
