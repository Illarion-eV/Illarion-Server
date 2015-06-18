set(GMOCK_DIR "/usr/src/gmock"
    CACHE PATH "Path to GoogleMock source directory")

add_subdirectory(${GMOCK_DIR} ${CMAKE_BINARY_DIR}/gmock)

include_directories(${CMAKE_SOURCE_DIR}/src)

add_executable(ItemTest ItemTest.cpp)
target_link_libraries(ItemTest server gmock_main)

find_package(Lua 5.2 REQUIRED)
include_directories(${LUA_INCLUDE_DIR})
#target_link_libraries(ItemTest ${LUA_LIBRARIES})

add_test(ItemTest ItemTest)