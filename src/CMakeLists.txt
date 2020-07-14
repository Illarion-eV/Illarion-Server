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

add_library(server
    data/ArmorObjectTable.cpp
    data/ArmorObjectTable.hpp
    data/ContainerObjectTable.cpp
    data/ContainerObjectTable.hpp
    data/Data.cpp
    data/Data.hpp
    data/ItemTable.cpp
    data/ItemTable.hpp
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
    map/Field.cpp
    map/Field.hpp
    map/Map.cpp
    map/Map.hpp
    map/WorldMap.cpp
    map/WorldMap.hpp
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
    script/binding/armor_struct.cpp
    script/binding/attack_boni.cpp
    script/binding/binding.hpp
    script/binding/character.cpp
    script/binding/character_skillvalue.cpp
    script/binding/colour.cpp
    script/binding/item_struct.cpp
    script/binding/container.cpp
    script/binding/crafting_dialog.cpp
    script/binding/field.cpp
    script/binding/input_dialog.cpp
    script/binding/item.cpp
    script/binding/item_look_at.cpp
    script/binding/long_time_action.cpp
    script/binding/long_time_character_effects.cpp
    script/binding/long_time_effect.cpp
    script/binding/long_time_effect_struct.cpp
    script/binding/merchant_dialog.cpp
    script/binding/message_dialog.cpp
    script/binding/monster.cpp
    script/binding/monster_armor.cpp
    script/binding/npc.cpp
    script/binding/player.cpp
    script/binding/position.cpp
    script/binding/random.cpp
    script/binding/script_item.cpp
    script/binding/script_variables_table.cpp
    script/binding/selection_dialog.cpp
    script/binding/tiles_struct.cpp
    script/binding/waypoint_list.cpp
    script/binding/weapon_struct.cpp
    script/binding/weather_struct.cpp
    script/binding/world.cpp
    script/forwarder.cpp
    script/forwarder.hpp
    script/LuaDepotScript.cpp
    script/LuaDepotScript.hpp
    script/LuaItemScript.cpp
    script/LuaItemScript.hpp
    script/LuaLearnScript.cpp
    script/LuaLearnScript.hpp
    script/LuaLoginScript.cpp
    script/LuaLoginScript.hpp
    script/LuaLogoutScript.cpp
    script/LuaLogoutScript.hpp
    script/LuaLongTimeEffectScript.cpp
    script/LuaLongTimeEffectScript.hpp
    script/LuaLookAtItemScript.cpp
    script/LuaLookAtItemScript.hpp
    script/LuaLookAtPlayerScript.cpp
    script/LuaLookAtPlayerScript.hpp
    script/LuaMagicScript.cpp
    script/LuaMagicScript.hpp
    script/LuaMonsterScript.cpp
    script/LuaMonsterScript.hpp
    script/LuaNPCScript.cpp
    script/LuaNPCScript.hpp
    script/LuaPlayerDeathScript.cpp
    script/LuaPlayerDeathScript.hpp
    script/LuaQuestScript.cpp
    script/LuaQuestScript.hpp
    script/LuaReloadScript.cpp
    script/LuaReloadScript.hpp
    script/LuaScheduledScript.cpp
    script/LuaScheduledScript.hpp
    script/LuaScript.cpp
    script/LuaScript.hpp
    script/LuaTestSupportScript.cpp
    script/LuaTestSupportScript.hpp
    script/LuaTileScript.cpp
    script/LuaTileScript.hpp
    script/LuaTriggerScript.cpp
    script/LuaTriggerScript.hpp
    script/LuaWeaponScript.cpp
    script/LuaWeaponScript.hpp
    script/server.cpp
    script/server.hpp
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
    constants.hpp
    Container.cpp
    Container.hpp
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
    Showcase.cpp
    Showcase.hpp
    SpawnPoint.cpp
    SpawnPoint.hpp
    stream.hpp
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
    WorldIMPLTools.cpp)

target_link_libraries(server db)

find_package(Lua 5.2 REQUIRED)
include_directories(${LUA_INCLUDE_DIR})
target_link_libraries(server ${LUA_LIBRARIES})
target_link_libraries(server /usr/lib/x86_64-linux-gnu/libluabind.so)

find_package(Boost 1.67.0 REQUIRED COMPONENTS system graph)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(server ${Boost_LIBRARIES})

find_package(Threads REQUIRED)
target_link_libraries(server ${CMAKE_THREAD_LIBS_INIT})
target_compile_options(server PRIVATE ${WARNINGS})

add_executable(illarion main.cpp)
target_link_libraries(illarion server)
target_link_libraries(illarion stdc++fs)

install (TARGETS illarion DESTINATION bin)

include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
set (CPACK_PACKAGE_VERSION_MAJOR "0")
set (CPACK_PACKAGE_VERSION_MINOR "9")
include (CPack)
