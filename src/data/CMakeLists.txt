find_package( Boost QUIET REQUIRED )
find_package( Filesystem REQUIRED )

add_library( data INTERFACE )
target_sources( data 
    INTERFACE
        ArmorObjectTable.cpp
        ContainerObjectTable.cpp
        Data.cpp
        ItemTable.cpp
        LongTimeEffectTable.cpp
        MonsterAttackTable.cpp
        MonsterTable.cpp
        NaturalArmorTable.cpp
        NPCTable.cpp
        QuestNodeTable.cpp
        QuestTable.cpp
        RaceTable.cpp
        RaceTypeTable.cpp
        ScheduledScriptsTable.cpp
        ScriptVariablesTable.cpp
        SkillTable.cpp
        SpellTable.cpp
        TilesModificatorTable.cpp
        TilesTable.cpp
        TriggerTable.cpp
        WeaponObjectTable.cpp
)

target_link_libraries( data INTERFACE db script )
target_link_libraries( data INTERFACE Boost::boost std::filesystem )
target_include_directories( data INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/.. )
target_compile_features( data INTERFACE cxx_std_20 )
