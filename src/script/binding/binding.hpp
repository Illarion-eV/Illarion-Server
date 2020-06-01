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

auto armor_struct() -> luabind::scope;
auto attack_boni() -> luabind::scope;
auto character() -> luabind::scope;
auto character_skillvalue() -> luabind::scope;
auto colour() -> luabind::scope;
auto item_struct() -> luabind::scope;
auto container() -> luabind::scope;
auto crafting_dialog() -> luabind::scope;
auto field() -> luabind::scope;
auto input_dialog() -> luabind::scope;
auto item() -> luabind::scope;
auto item_look_at() -> luabind::scope;
auto long_time_action() -> luabind::scope;
auto long_time_character_effects() -> luabind::scope;
auto long_time_effect() -> luabind::scope;
auto long_time_effect_struct() -> luabind::scope;
auto merchant_dialog() -> luabind::scope;
auto message_dialog() -> luabind::scope;
auto monster() -> luabind::scope;
auto monster_armor() -> luabind::scope;
auto npc() -> luabind::scope;
auto player() -> luabind::scope;
auto position() -> luabind::scope;
auto random() -> luabind::scope;
auto script_item() -> luabind::scope;
auto script_variables_table() -> luabind::scope;
auto selection_dialog() -> luabind::scope;
auto tiles_struct() -> luabind::scope;
auto waypoint_list() -> luabind::scope;
auto weapon_struct() -> luabind::scope;
auto weather_struct() -> luabind::scope;
auto world() -> luabind::scope;

} // namespace binding