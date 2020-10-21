find_package( Boost 1.67.0 REQUIRED )
find_package( Filesystem REQUIRED )

add_library( map INTERFACE )
target_sources( map 
    INTERFACE
        Field.cpp
        Map.cpp
        WorldMap.cpp
)

target_link_libraries( map INTERFACE db data )
target_link_libraries( map INTERFACE Boost::boost std::filesystem )
target_include_directories( map INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/.. )
target_compile_features( map INTERFACE cxx_std_17 )
