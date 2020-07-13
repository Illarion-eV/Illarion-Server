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

#include "script/forwarder.hpp"

#include "Logger.hpp"
#include "Monster.hpp"
#include "NPC.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "globals.hpp"
#include "map/Field.hpp"
#include "script/LuaScript.hpp"

auto convert_to_map(const luabind::object &data) -> std::shared_ptr<script_data_exchangemap> {
    auto mapType = luabind::type(data);

    if (mapType == LUA_TNIL) {
        return {};
    }

    if (mapType != LUA_TTABLE) {
        throw std::logic_error("Usage of invalid data map type. Data maps must be tables or nil.");
    }

    luabind::iterator end;

    auto result = std::make_shared<script_data_exchangemap>();

    for (auto it = luabind::iterator(data); it != end; ++it) {
        std::string key;

        try {
            key = luabind::object_cast<std::string>(it.key());
        } catch (luabind::cast_failed &) {
            throw std::logic_error("Usage of invalid data map key. Data map keys must be strings.");
        }

        std::string value;

        try {
            value = luabind::object_cast<std::string>(*it);
        } catch (luabind::cast_failed &) {
            try {
                auto intValue = luabind::object_cast<int32_t>(*it);
                std::stringstream ss;
                ss << intValue;
                value = ss.str();
            } catch (luabind::cast_failed &) {
                throw std::logic_error("Usage of invalid data map value. Data map values must be numbers or strings.");
            }
        }

        result->push_back(std::make_pair(key, value));
    }

    return result;
}

auto getPlayerLanguageLua(const Character *character) -> uint32_t {
    return static_cast<uint32_t>(character->getPlayerLanguage());
}

void inform_lua2(const Character *character, const std::string &message, Character::informType type) {
    switch (type) {
    case Character::informScriptLowPriority:
    case Character::informScriptMediumPriority:
    case Character::informScriptHighPriority:
        break;

    default:
        type = Character::informScriptMediumPriority;
    }

    character->inform(message, type);
}

void inform_lua4(const Character *character, const std::string &message) {
    character->inform(message, Character::informScriptMediumPriority);
}

void inform_lua1(const Character *character, const std::string &german, const std::string &english,
                 Character::informType type) {
    switch (type) {
    case Character::informScriptLowPriority:
    case Character::informScriptMediumPriority:
    case Character::informScriptHighPriority:
        break;

    default:
        type = Character::informScriptMediumPriority;
    }

    character->inform(german, english, type);
}

void inform_lua3(const Character *character, const std::string &german, const std::string &english) {
    character->inform(german, english, Character::informScriptMediumPriority);
}

auto count_item_at1(const Character *character, const std::string &where, TYPE_OF_ITEM_ID id) -> int {
    return character->countItemAt(where, id);
}

auto count_item_at2(const Character *character, const std::string &where, TYPE_OF_ITEM_ID id,
                    const luabind::object &data) -> int {
    return character->countItemAt(where, id, convert_to_map(data).get());
}

auto erase_item1(Character *character, TYPE_OF_ITEM_ID id, int count) -> int { return character->eraseItem(id, count); }

auto erase_item2(Character *character, TYPE_OF_ITEM_ID id, int count, const luabind::object &data) -> int {
    return character->eraseItem(id, count, convert_to_map(data).get());
}

auto create_item(Character *character, Item::id_type id, Item::number_type number, Item::quality_type quality,
                 const luabind::object &data) -> int {
    return character->createItem(id, number, quality, convert_to_map(data).get());
}

auto getLoot(const Character *character) -> luabind::object {
    lua_State *_luaState = LuaScript::getLuaState();
    luabind::object lootTable = luabind::newtable(_luaState);

    try {
        const auto &loot = character->getLoot();

        for (const auto &category : loot) {
            const auto categoryId = category.first;
            const auto &categoryItems = category.second;

            luabind::object lootTableCategory = luabind::newtable(_luaState);

            for (const auto &item : categoryItems) {
                const auto lootId = item.first;
                const auto &lootItem = item.second;
                // auto &lootTableItem = lootTable[categoryId][lootId];

                // probability, itemId, minAmount, maxAmount, minQuality, maxQuality, minDurability, maxDurability
                luabind::object lootTableItem = luabind::newtable(_luaState);
                lootTableItem["probability"] = lootItem.probability;
                lootTableItem["itemId"] = lootItem.itemId;
                lootTableItem["minAmount"] = lootItem.amount.first;
                lootTableItem["maxAmount"] = lootItem.amount.second;
                lootTableItem["minQuality"] = lootItem.quality.first;
                lootTableItem["maxQuality"] = lootItem.quality.second;
                lootTableItem["minDurability"] = lootItem.durability.first;
                lootTableItem["maxDurability"] = lootItem.durability.second;

                luabind::object lootTableItemData = luabind::newtable(_luaState);

                for (const auto &data : lootItem.data) {
                    lootTableItemData[data.first] = data.second;
                }

                lootTableItem["data"] = lootTableItemData;
                lootTableCategory[lootId] = lootTableItem;
            }

            lootTable[categoryId] = lootTableCategory;
        }
    } catch (NoLootFound &) {
    }

    return lootTable;
}

auto container_count_item1(Container *container, Item::id_type id) -> int { return container->countItem(id); }

auto container_count_item2(Container *container, Item::id_type id, const luabind::object &data) -> int {
    return container->countItem(id, convert_to_map(data).get());
}

auto container_erase_item1(Container *container, Item::id_type id, Item::number_type number) -> int {
    return container->eraseItem(id, number);
}

auto container_erase_item2(Container *container, Item::id_type id, Item::number_type number,
                           const luabind::object &data) -> int {
    return container->eraseItem(id, number, convert_to_map(data).get());
}

auto world_fieldAt(World *world, const position &pos) -> map::Field * {
    try {
        return &world->fieldAt(pos);
    } catch (FieldNotFound &) {
        return nullptr;
    }
}

auto world_createFromId(World *world, TYPE_OF_ITEM_ID id, unsigned short int count, position pos, bool always,
                        int quali, const luabind::object &data) -> ScriptItem {
    return world->createFromId(id, count, pos, always, quali, convert_to_map(data).get());
}

void log_lua(const std::string &message) { Logger::info(LogFacility::Script) << message << Log::end; }

auto character_getItemList(Character *character, TYPE_OF_ITEM_ID id) -> luabind::object {
    auto items = character->getItemList(id);
    lua_State *_luaState = LuaScript::getLuaState();
    luabind::object list = luabind::newtable(_luaState);

    int index = 1;

    for (const auto &item : items) {
        list[index++] = item;
    }

    return list;
}

auto waypointlist_getWaypoints(const WaypointList *wpl) -> luabind::object {
    lua_State *luaState = LuaScript::getLuaState();
    luabind::object list = luabind::newtable(luaState);

    int index = 1;

    const auto &waypoints = wpl->getWaypoints();

    for (const auto &waypoint : waypoints) {
        list[index++] = waypoint;
    }

    return list;
}

void waypointlist_addFromList(WaypointList *wpl, const luabind::object &list) {
    if (list.is_valid()) {
        if (luabind::type(list) == LUA_TTABLE) {
            for (luabind::iterator it(list), end; it != end; ++it) {
                try {
                    auto pos = luabind::object_cast<position>(*it);
                    wpl->addWaypoint(pos);
                } catch (luabind::cast_failed &e) {
                    const std::string script = World::get()->getCurrentScript()->getFileName();
                    Logger::error(LogFacility::Script)
                            << "Invalid type in parameter list of WaypointList:addFromList in " << script << ": "
                            << "Expected type position" << Log::end;
                }
            }
        }
    }
}

auto world_LuaLoS(const World *world, const position &startingpos, const position &endingpos) -> luabind::object {
    lua_State *luaState = LuaScript::getLuaState();
    luabind::object list = luabind::newtable(luaState);
    int index = 1;
    auto objects = world->blockingLineOfSight(startingpos, endingpos);

    for (auto &blocker : objects) {
        luabind::object innerlist = luabind::newtable(luaState);

        if (blocker.blockingType == BlockingObject::BT_CHARACTER) {
            innerlist["TYPE"] = "CHARACTER";
            innerlist["OBJECT"] = character_ptr(blocker.blockingChar);
        } else if (blocker.blockingType == BlockingObject::BT_ITEM) {
            innerlist["TYPE"] = "ITEM";
            innerlist["OBJECT"] = blocker.blockingItem;
        }

        list[index] = innerlist;
        index++;
    }

    return list;
}

auto world_getPlayersOnline(const World *world) -> luabind::object {
    lua_State *luaState = LuaScript::getLuaState();
    luabind::object list = luabind::newtable(luaState);
    int index = 1;

    const auto &players = world->getPlayersOnline();

    for (auto *player : players) {
        list[index] = character_ptr(player);
        index++;
    }

    return list;
}

template <typename Container> auto convert_to_fuselist(const Container &container) -> luabind::object {
    lua_State *luaState = LuaScript::getLuaState();
    luabind::object list = luabind::newtable(luaState);
    int index = 1;

    for (auto item : container) {
        list[index] = character_ptr(item);
        index++;
    }

    return list;
}

auto world_getNPCS(const World *world) -> luabind::object { return convert_to_fuselist(world->getNPCS()); }

auto world_getCharactersInRangeOf(const World *world, const position &posi, uint8_t range) -> luabind::object {
    return convert_to_fuselist(world->getCharactersInRangeOf(posi, range));
}

auto world_getPlayersInRangeOf(const World *world, const position &posi, uint8_t range) -> luabind::object {
    return convert_to_fuselist(world->getPlayersInRangeOf(posi, range));
}

auto world_getMonstersInRangeOf(const World *world, const position &posi, uint8_t range) -> luabind::object {
    return convert_to_fuselist(world->getMonstersInRangeOf(posi, range));
}

auto world_getNPCSInRangeOf(const World *world, const position &posi, uint8_t range) -> luabind::object {
    return convert_to_fuselist(world->getNPCSInRangeOf(posi, range));
}
