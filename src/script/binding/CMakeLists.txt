find_package( Luabind REQUIRED )

add_library( script_binding INTERFACE )
target_sources( script_binding 
    INTERFACE 
        armor_struct.cpp
        attack_boni.cpp
        character.cpp
        character_skillvalue.cpp
        colour.cpp
        container.cpp
        crafting_dialog.cpp
        field.cpp
        input_dialog.cpp
        item.cpp
        item_look_at.cpp
        item_struct.cpp
        long_time_action.cpp
        long_time_character_effects.cpp
        long_time_effect.cpp
        long_time_effect_struct.cpp
        merchant_dialog.cpp
        message_dialog.cpp
        monster.cpp
        monster_armor.cpp
        npc.cpp
        player.cpp
        position.cpp
        random.cpp
        script_item.cpp
        script_variables_table.cpp
        selection_dialog.cpp
        tiles_struct.cpp
        waypoint_list.cpp
        weapon_struct.cpp
        weather_struct.cpp
        world.cpp
)

target_link_libraries( script_binding INTERFACE data dialog server )
target_link_libraries( script_binding INTERFACE Luabind::Luabind range-v3::range-v3 )
target_include_directories( script_binding INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../.. )
target_compile_features( script_binding INTERFACE cxx_std_20 )
