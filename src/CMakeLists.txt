find_package( Boost 1.67.0 REQUIRED COMPONENTS system graph )
find_package( Filesystem REQUIRED )
find_package( Pqxx 6.2 REQUIRED )
find_package( Lua 5.2 EXACT REQUIRED )
find_package( Luabind REQUIRED )
find_package( Threads REQUIRED )

add_subdirectory( data )
add_subdirectory( db )
add_subdirectory( dialog )
add_subdirectory( map )
add_subdirectory( netinterface )
add_subdirectory( script )

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.hpp"
)

add_compile_definitions(LUABIND_USE_CXX11)

add_library( server STATIC "" )

target_sources( server 
    PRIVATE
        Attribute.cpp
        a_star.cpp
        Character.cpp
        CharacterContainer.cpp
        character_ptr.cpp
        Config.cpp
        Container.cpp
        InitialConnection.cpp
        Item.cpp
        Logger.cpp
        LongTimeAction.cpp
        LongTimeCharacterEffects.cpp
        LongTimeEffect.cpp
        main_help.cpp
        MonitoringClients.cpp
        Monster.cpp
        NewClientView.cpp
        NPC.cpp
        Player.cpp
        PlayerManager.cpp
        PlayerWorkoutCommands.cpp
        Random.cpp
        Showcase.cpp
        SpawnPoint.cpp
        Timer.cpp
        utility.cpp
        WaypointList.cpp
        World.cpp
        WorldIMPLAdmin.cpp
        WorldIMPLCharacterMoves.cpp
        WorldIMPLItemMoves.cpp
        WorldIMPLPlayer.cpp
        WorldIMPLScriptHelp.cpp
        WorldIMPLTalk.cpp
        WorldIMPLTools.cpp
)

target_link_libraries( server PRIVATE data dialog map netinterface script )
target_link_libraries( server PUBLIC db )
target_link_libraries( server PUBLIC Boost::system Boost::graph Threads::Threads range-v3::range-v3 )
# Additional links for the private interface libraries
target_link_libraries( server PUBLIC Luabind::Luabind Pqxx::Pqxx std::filesystem )

target_include_directories( server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )
target_compile_features( server PUBLIC cxx_std_17 )

add_executable( illarion main.cpp )
target_link_libraries( illarion PRIVATE server )
target_compile_features( illarion PRIVATE cxx_std_17 )

include( GNUInstallDirs )
install(TARGETS illarion
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                    GROUP_EXECUTE
                    WORLD_EXECUTE)
install(FILES ../setup/illarion.conf
        DESTINATION /etc
        PERMISSIONS OWNER_WRITE OWNER_READ
                                GROUP_READ
                                WORLD_READ)

include( InstallRequiredSystemLibraries )
set( CPACK_GENERATOR "DEB" )
set( CPACK_PROJECT_CONFIG_FILE "${CMAKE_SOURCE_DIR}/CPackConfig.cmake" )
include( CPack )
