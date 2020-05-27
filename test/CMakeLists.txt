set(GMOCK_DIR "/usr/src/gmock"
    CACHE PATH "Path to GoogleMock source directory")

add_subdirectory(${GMOCK_DIR} ${CMAKE_BINARY_DIR}/gmock)

include_directories(${CMAKE_SOURCE_DIR}/src)
find_package(Lua 5.2 REQUIRED)
include_directories(${LUA_INCLUDE_DIR})

file(COPY maps/* DESTINATION ${CMAKE_BINARY_DIR}/test/data/map/import)

macro (run_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} server gmock_main)
    add_test(${name} ${name})
endmacro(run_test)

run_test(CharacterContainerTest)
run_test(ItemTest)
run_test(test_binding)
run_test(test_binding_character)
run_test(test_binding_item)
run_test(test_binding_longtimeaction)
run_test(test_binding_position)
run_test(test_binding_scriptitem)
run_test(test_binding_weatherstruct)
run_test(test_container)
run_test(test_map_import)
