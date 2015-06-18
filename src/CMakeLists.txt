find_package(Git)
if (GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --long
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        OUTPUT_VARIABLE illarion_VERSION
        RESULT_VARIABLE version_error
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (version_error)
        set(illarion_VERSION "unversioned")
    endif (version_error)
else()
    set(illarion_VERSION "unknown")
endif()

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.hpp"
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/db)
add_subdirectory(db)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/script)
add_subdirectory(script)

add_library(server
    data/ArmorObjectTable.cpp
    data/ArmorObjectTable.hpp
    data/CommonObjectTable.cpp
    data/CommonObjectTable.hpp
    data/ContainerObjectTable.cpp
    data/ContainerObjectTable.hpp
    data/Data.cpp
    data/Data.hpp
    data/LongTimeEffectTable.cpp
    data/LongTimeEffectTable.hpp
    data/MonsterAttackTable.cpp
    data/MonsterAttackTable.hpp
    data/MonsterTable.cpp
    data/MonsterTable.hpp
    data/NaturalArmorTable.cpp
    data/NaturalArmorTable.hpp
    data/NPCTable.cpp
    data/NPCTable.hpp
    data/QuestNodeTable.cpp
    data/QuestNodeTable.hpp
    data/QuestScriptStructTable.hpp
    data/QuestTable.cpp
    data/QuestTable.hpp
    data/RaceTable.cpp
    data/RaceTable.hpp
    data/RaceTypeTable.cpp
    data/RaceTypeTable.hpp
    data/ScheduledScriptsTable.cpp
    data/ScheduledScriptsTable.hpp
    data/ScriptStructTable.hpp
    data/ScriptVariablesTable.cpp
    data/ScriptVariablesTable.hpp
    data/SkillTable.cpp
    data/SkillTable.hpp
    data/SpellTable.cpp
    data/SpellTable.hpp
    data/StructTable.hpp
    data/Table.hpp
    data/TilesModificatorTable.cpp
    data/TilesModificatorTable.hpp
    data/TilesTable.cpp
    data/TilesTable.hpp
    data/TriggerTable.cpp
    data/TriggerTable.hpp
    data/WeaponObjectTable.cpp
    data/WeaponObjectTable.hpp
    dialog/CraftingDialog.cpp
    dialog/CraftingDialog.hpp
    dialog/Dialog.cpp
    dialog/Dialog.hpp
    dialog/InputDialog.cpp
    dialog/InputDialog.hpp
    dialog/MerchantDialog.cpp
    dialog/MerchantDialog.hpp
    dialog/MessageDialog.cpp
    dialog/MessageDialog.hpp
    dialog/SelectionDialog.cpp
    dialog/SelectionDialog.hpp
    netinterface/protocol/BBIWIClientCommands.cpp
    netinterface/protocol/BBIWIClientCommands.hpp
    netinterface/protocol/BBIWIServerCommands.cpp
    netinterface/protocol/BBIWIServerCommands.hpp
    netinterface/protocol/ClientCommands.cpp
    netinterface/protocol/ClientCommands.hpp
    netinterface/protocol/ServerCommands.cpp
    netinterface/protocol/ServerCommands.hpp
    netinterface/BasicClientCommand.cpp
    netinterface/BasicClientCommand.hpp
    netinterface/BasicCommand.cpp
    netinterface/BasicCommand.hpp
    netinterface/BasicServerCommand.cpp
    netinterface/BasicServerCommand.hpp
    netinterface/ByteBuffer.cpp
    netinterface/ByteBuffer.hpp
    netinterface/CommandFactory.cpp
    netinterface/CommandFactory.hpp
    netinterface/NetInterface.cpp
    netinterface/NetInterface.hpp
    a_star.cpp
    a_star.hpp
    Attribute.cpp
    Attribute.hpp
    Character.cpp
    Character.hpp
    character_ptr.cpp
    character_ptr.hpp
    CharacterContainer.cpp
    CharacterContainer.hpp
    Config.cpp
    Config.hpp
    Connection.hpp
    constants.hpp
    Container.cpp
    Container.hpp
    Field.cpp
    Field.hpp
    globals.hpp
    InitialConnection.cpp
    InitialConnection.hpp
    Item.cpp
    Item.hpp
    ItemLookAt.hpp
    Language.hpp
    Logger.cpp
    Logger.hpp
    LongTimeAction.cpp
    LongTimeAction.hpp
    LongTimeCharacterEffects.cpp
    LongTimeCharacterEffects.hpp
    LongTimeEffect.cpp
    LongTimeEffect.hpp
    main_help.cpp
    main_help.hpp
    Map.cpp
    Map.hpp
    MilTimer.cpp
    MilTimer.hpp
    MonitoringClients.cpp
    MonitoringClients.hpp
    Monster.cpp
    Monster.hpp
    NewClientView.cpp
    NewClientView.hpp
    NPC.cpp
    NPC.hpp
    Player.cpp
    Player.hpp
    PlayerManager.cpp
    PlayerManager.hpp
    PlayerWorkoutCommands.cpp
    Random.cpp
    Random.hpp
    Scheduler.hpp
    Scheduler.tcc
    SchedulerTaskClasses.hpp
    Showcase.cpp
    Showcase.hpp
    SpawnPoint.cpp
    SpawnPoint.hpp
    Statistics.cpp
    Statistics.hpp
    TableStructs.hpp
    thread_safe_vector.hpp
    Timer.cpp
    Timer.hpp
    tuningConstants.hpp
    types.hpp
    utility.cpp
    utility.hpp
    WaypointList.cpp
    WaypointList.hpp
    World.cpp
    World.hpp
    WorldIMPLAdmin.cpp
    WorldIMPLCharacterMoves.cpp
    WorldIMPLItemMoves.cpp
    WorldIMPLPlayer.cpp
    WorldIMPLScriptHelp.cpp
    WorldIMPLTalk.cpp
    WorldIMPLTools.cpp
    WorldMap.cpp
    WorldMap.hpp)

target_link_libraries(server db script)

find_package(Lua 5.2 REQUIRED)
include_directories(${LUA_INCLUDE_DIR})

find_package(Boost 1.55.0 REQUIRED COMPONENTS filesystem system graph)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(server ${Boost_LIBRARIES})

find_package(Threads REQUIRED)
target_link_libraries(server ${CMAKE_THREAD_LIBS_INIT})

add_executable(illarion main.cpp)
target_link_libraries(illarion server)

install (TARGETS illarion DESTINATION bin)

include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
set (CPACK_PACKAGE_VERSION_MAJOR "0")
set (CPACK_PACKAGE_VERSION_MINOR "9")
include (CPack)