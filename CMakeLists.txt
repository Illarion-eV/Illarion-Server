cmake_minimum_required(VERSION 3.0)
project(illarion)

enable_testing()
include(GoogleTest)

set(CMAKE_CXX_FLAGS "-DLUABIND_USE_CXX11 ${CMAKE_CXX_FLAGS} -std=c++17")
set(WARNINGS -Wall -Wformat-security -Wunused-result -Werror -Wno-deprecated -pedantic-errors)

add_subdirectory(src)
add_subdirectory(test)
