include( Dart )

file( GLOB MapTestFiles ${CMAKE_CURRENT_SOURCE_DIR}/maps/*.txt )

add_custom_target( CopyMapTestFiles
                   COMMENT "Copy the files that are required for the map unit tests."
                   DEPENDS ${MapTestFiles} )
foreach( MapTestFile ${MapTestFiles} )
  get_filename_component( MapTestFileName ${MapTestFile} NAME )
  add_custom_command( TARGET CopyMapTestFiles
                      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MapTestFile} ${CMAKE_CURRENT_BINARY_DIR}/data/map/import/${MapTestFileName} )
endforeach()

function( run_test )
    set( multiValueArgs DEPENDENCIES )
    cmake_parse_arguments( run_test "" "" "${multiValueArgs}" ${ARGN} )

    set( name ${run_test_UNPARSED_ARGUMENTS} )
    add_executable( ${name} "" )
    target_sources( ${name} PRIVATE ${name}.cpp )
    target_link_libraries( ${name} PRIVATE server )
    target_link_libraries( ${name} PRIVATE gtest gmock gmock_main )
    target_compile_features( ${name} PRIVATE cxx_std_20 )

    if( NOT "${run_test_DEPENDENCIES}" STREQUAL "" )
        add_dependencies( ${name} ${run_test_DEPENDENCIES} )
    endif()
    add_test( ${name} ${name} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
endfunction()

run_test( CharacterContainerTest )
run_test( ItemTest )
run_test( test_binding )
run_test( test_binding_armorstruct )
run_test( test_binding_character )
run_test( test_binding_item )
run_test( test_binding_longtimeaction )
run_test( test_binding_position )
run_test( test_binding_random )
run_test( test_binding_scriptitem )
run_test( test_binding_weatherstruct )
run_test( test_binding_world )
run_test( test_container )
run_test( test_map_import DEPENDENCIES CopyMapTestFiles )
run_test( test_random )
run_test( test_timer )
