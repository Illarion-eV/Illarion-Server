find_package( Boost QUIET REQUIRED )
find_package( Luabind REQUIRED )

add_subdirectory( binding )

add_library( script INTERFACE )
target_sources( script 
    INTERFACE 
        forwarder.cpp
        LuaDepotScript.cpp
        LuaItemScript.cpp
        LuaLearnScript.cpp
        LuaLoginScript.cpp
        LuaLogoutScript.cpp
        LuaLongTimeEffectScript.cpp
        LuaLookAtItemScript.cpp
        LuaLookAtPlayerScript.cpp
        LuaMagicScript.cpp
        LuaMonsterScript.cpp
        LuaNPCScript.cpp
        LuaPlayerDeathScript.cpp
        LuaQuestScript.cpp
        LuaReloadScript.cpp
        LuaScheduledScript.cpp
        LuaScript.cpp
        LuaTestSupportScript.cpp
        LuaTileScript.cpp
        LuaTriggerScript.cpp
        LuaWeaponScript.cpp
        server.cpp
)

target_link_libraries( script INTERFACE script_binding map )
target_link_libraries( script INTERFACE Boost::boost Luabind::Luabind )
target_include_directories( script INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/.. )
target_compile_features( script INTERFACE  cxx_std_17 )
