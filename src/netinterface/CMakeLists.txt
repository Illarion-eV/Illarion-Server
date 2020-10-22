find_package( Boost REQUIRED )

add_subdirectory( protocol )

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
