/*
 *  illarionserver - server for the game Illarion
 *  Copyright 2011 Illarion e.V.
 *
 *  This file is part of illarionserver.
 *
 *  illarionserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  illarionserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "luabind/luabind.hpp"

namespace binding {

    luabind::scope armor_struct();
    luabind::scope attack_boni();
    luabind::scope character();
    luabind::scope character_skillvalue();
    luabind::scope colour();
    luabind::scope item_struct();
    luabind::scope container();
    luabind::scope crafting_dialog();
    luabind::scope field();
    luabind::scope input_dialog();
    luabind::scope item();
    luabind::scope item_look_at();
    luabind::scope long_time_action();
    luabind::scope long_time_character_effects();
    luabind::scope long_time_effect();
    luabind::scope long_time_effect_struct();
    luabind::scope merchant_dialog();
    luabind::scope message_dialog();
    luabind::scope monster();
    luabind::scope monster_armor();
    luabind::scope npc();
    luabind::scope player();
    luabind::scope position();
    luabind::scope random();
    luabind::scope script_item();
    luabind::scope script_variables_table();
    luabind::scope selection_dialog();
    luabind::scope tiles_struct();
    luabind::scope waypoint_list();
    luabind::scope weapon_struct();
    luabind::scope weather_struct();
    luabind::scope world();

}