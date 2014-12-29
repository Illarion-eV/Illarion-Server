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

#ifndef _SCRIPT_FOWARDER_HPP_
#define _SCRIPT_FOWARDER_HPP_

#include <stdint.h>
#include <string>
#include "Character.hpp"
#include "Container.hpp"
#include <luabind/object.hpp>

uint32_t getPlayerLanguageLua(const Character *);

void inform_lua2(const Character *character, const std::string &message, Character::informType type);
void inform_lua4(const Character *character, const std::string &message);
void inform_lua1(const Character *character, const std::string &german, const std::string &english, Character::informType type);
void inform_lua3(const Character *character, const std::string &german, const std::string &english);

int count_item_at1(const Character *, const std::string &where, TYPE_OF_ITEM_ID id);
int count_item_at2(const Character *, const std::string &where, TYPE_OF_ITEM_ID id, const luabind::object &);

int erase_item1(Character *, TYPE_OF_ITEM_ID, int);
int erase_item2(Character *, TYPE_OF_ITEM_ID, int, const luabind::object &);

int create_item(Character *, Item::id_type id, Item::number_type number, Item::quality_type quality, const luabind::object &);

luabind::object getLoot(const Character *character);

int container_count_item1(Container *, Item::id_type);
int container_count_item2(Container *, Item::id_type, const luabind::object &data);

int container_erase_item1(Container *, Item::id_type, Item::number_type);
int container_erase_item2(Container *, Item::id_type, Item::number_type, const luabind::object &data);

Field *world_fieldAt(World *world, const position &pos);
ScriptItem world_createFromId(World *world, TYPE_OF_ITEM_ID id, unsigned short int count, position pos, bool allways, int quali, const luabind::object &data);

void log_lua(const std::string &message);

luabind::object character_getItemList(const Character *, TYPE_OF_ITEM_ID id);

void waypointlist_addFromList(WaypointList *wpl, const luabind::object &list);
luabind::object waypointlist_getWaypoints(const WaypointList *wpl);

luabind::object world_LuaLoS(const World *world, const position &startingpos, const position &endingpos);
luabind::object world_getPlayersOnline(const World *world);
luabind::object world_getNPCS(const World *world);

luabind::object world_getCharactersInRangeOf(const World *world, const position &posi, uint8_t range);
luabind::object world_getPlayersInRangeOf(const World *world, const position &posi, uint8_t range);
luabind::object world_getMonstersInRangeOf(const World *world, const position &posi, uint8_t range);
luabind::object world_getNPCSInRangeOf(const World *world, const position &posi, uint8_t range);

#endif
