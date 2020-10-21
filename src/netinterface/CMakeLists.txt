add_subdirectory( protocol )

find_package( Boost 1.67.0 REQUIRED )

add_library( netinterface INTERFACE )
target_sources( netinterface 
    INTERFACE
        BasicClientCommand.cpp
        BasicCommand.cpp
        BasicServerCommand.cpp
        ByteBuffer.cpp
        CommandFactory.cpp
        NetInterface.cpp
)

target_link_libraries( netinterface INTERFACE netinterface_protocol )
target_link_libraries( netinterface INTERFACE Boost::boost )
target_include_directories( netinterface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/.. )
target_compile_features( netinterface INTERFACE cxx_std_17 )
