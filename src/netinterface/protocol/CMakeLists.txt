add_library( netinterface_protocol INTERFACE )
target_sources( netinterface_protocol 
INTERFACE
        BBIWIClientCommands.cpp
        BBIWIServerCommands.cpp
        ClientCommands.cpp
        ServerCommands.cpp
)

target_link_libraries( netinterface_protocol INTERFACE data dialog map script )
target_link_libraries( netinterface_protocol INTERFACE range-v3::range-v3 )
target_include_directories( netinterface_protocol INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../.. )
target_compile_features( netinterface_protocol INTERFACE cxx_std_20 )
