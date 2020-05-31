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

#ifndef SCRIPT_FOWARDER_HPP
#define SCRIPT_FOWARDER_HPP

#include "Character.hpp"
#include "Container.hpp"
#include <cstdint>
#include <luabind/object.hpp>
#include <string>

namespace map {
    class Field;
}

auto getPlayerLanguageLua(const Character *) -> uint32_t;

void inform_lua2(const Character *character, const std::string &message, Character::informType type);
void inform_lua4(const Character *character, const std::string &message);
void inform_lua1(const Character *character, const std::string &german, const std::string &english, Character::informType type);
void inform_lua3(const Character *character, const std::string &german, const std::string &english);

auto count_item_at1(const Character *, const std::string &where, TYPE_OF_ITEM_ID id) -> int;
auto count_item_at2(const Character *, const std::string &where, TYPE_OF_ITEM_ID id, const luabind::object &) -> int;

auto erase_item1(Character *, TYPE_OF_ITEM_ID, int) -> int;
auto erase_item2(Character *, TYPE_OF_ITEM_ID, int, const luabind::object &) -> int;

auto create_item(Character *, Item::id_type id, Item::number_type number, Item::quality_type quality, const luabind::object &) -> int;

auto getLoot(const Character *character) -> luabind::object;

auto container_count_item1(Container *, Item::id_type) -> int;
auto container_count_item2(Container *, Item::id_type, const luabind::object &data) -> int;

auto container_erase_item1(Container *, Item::id_type, Item::number_type) -> int;
auto container_erase_item2(Container *, Item::id_type, Item::number_type, const luabind::object &data) -> int;

auto world_fieldAt(World *world, const position &pos) -> map::Field *;
auto world_createFromId(World *world, TYPE_OF_ITEM_ID id, unsigned short int count, position pos, bool always, int quali, const luabind::object &data) -> ScriptItem;

void log_lua(const std::string &message);

auto character_getItemList(const Character *, TYPE_OF_ITEM_ID id) -> luabind::object;

void waypointlist_addFromList(WaypointList *wpl, const luabind::object &list);
auto waypointlist_getWaypoints(const WaypointList *wpl) -> luabind::object;

auto world_LuaLoS(const World *world, const position &startingpos, const position &endingpos) -> luabind::object;
auto world_getPlayersOnline(const World *world) -> luabind::object;
auto world_getNPCS(const World *world) -> luabind::object;

auto world_getCharactersInRangeOf(const World *world, const position &posi, uint8_t range) -> luabind::object;
auto world_getPlayersInRangeOf(const World *world, const position &posi, uint8_t range) -> luabind::object;
auto world_getMonstersInRangeOf(const World *world, const position &posi, uint8_t range) -> luabind::object;
auto world_getNPCSInRangeOf(const World *world, const position &posi, uint8_t range) -> luabind::object;

#endif
