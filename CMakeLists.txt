cmake_minimum_required(VERSION 3.0)
project(illarion)

enable_testing()

set(CMAKE_CXX_FLAGS "-DLUABIND_USE_CXX11 ${CMAKE_CXX_FLAGS} -std=c++17")

add_subdirectory(src)
add_subdirectory(test)
