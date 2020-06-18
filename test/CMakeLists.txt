add_subdirectory("${PROJECT_SOURCE_DIR}/extern/googletest" "extern/googletest")

mark_as_advanced(
    BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
    gmock_build_tests gtest_build_samples gtest_build_tests
    gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
)

include_directories(${CMAKE_SOURCE_DIR}/src)
find_package(Lua 5.2 REQUIRED)
include_directories(${LUA_INCLUDE_DIR})

file(COPY maps DESTINATION ${CMAKE_BINARY_DIR}/test/data/map/import)

macro (run_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} server db script gtest gmock gmock_main stdc++fs)
    add_test(${name} ${name})
endmacro(run_test)

run_test(CharacterContainerTest)
run_test(ItemTest)
run_test(test_binding)
run_test(test_binding_armorstruct)
run_test(test_binding_character)
run_test(test_binding_item)
run_test(test_binding_longtimeaction)
run_test(test_binding_position)
run_test(test_binding_random)
run_test(test_binding_scriptitem)
run_test(test_binding_weatherstruct)
run_test(test_binding_world)
run_test(test_container)
run_test(test_map_import)
run_test(test_random)
