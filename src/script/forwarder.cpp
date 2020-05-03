#include "script/forwarder.hpp"

#include "globals.hpp"
#include "World.hpp"
#include "Logger.hpp"
#include "Player.hpp"
#include "Monster.hpp"
#include "NPC.hpp"

#include "script/LuaScript.hpp"

std::shared_ptr<script_data_exchangemap> convert_to_map(const luabind::object &data) {
    auto mapType = luabind::type(data);

    if (mapType == LUA_TNIL)
        return {};

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
                int32_t intValue = luabind::object_cast<int32_t>(*it);
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

uint32_t getPlayerLanguageLua(const Character *character) {
    return static_cast<uint32_t>(character->getPlayerLanguage());
}

void inform_lua2(const Character *character, const std::string &message, Character::informType type) {
    switch (type) {
    case Character::informScriptLowPriority:
    case Character::informScriptMediumPriority:
    case Character::informScriptHighPriority:
        break;

    default:
        type=Character::informScriptMediumPriority;
    }

    character->inform(message, type);
}

void inform_lua4(const Character *character, const std::string &message) {
    character->inform(message, Character::informScriptMediumPriority);
}

void inform_lua1(const Character *character, const std::string &german, const std::string &english, Character::informType type) {
    switch (type) {
    case Character::informScriptLowPriority:
    case Character::informScriptMediumPriority:
    case Character::informScriptHighPriority:
        break;

    default:
        type=Character::informScriptMediumPriority;
    }

    character->inform(german, english, type);
}

void inform_lua3(const Character *character, const std::string &german, const std::string &english) {
    character->inform(german, english, Character::informScriptMediumPriority);
}

int count_item_at1(const Character *character, const std::string &where, TYPE_OF_ITEM_ID id) {
    return character->countItemAt(where, id);
}

int count_item_at2(const Character *character, const std::string &where, TYPE_OF_ITEM_ID id, const luabind::object &data) {
    return character->countItemAt(where, id, convert_to_map(data).get());
}


int erase_item1(Character *character, TYPE_OF_ITEM_ID id, int count) {
    return character->eraseItem(id, count);
}

int erase_item2(Character *character, TYPE_OF_ITEM_ID id, int count, const luabind::object &data) {
    return character->eraseItem(id, count, convert_to_map(data).get());
}

int create_item(Character *character, Item::id_type id, Item::number_type number, Item::quality_type quality, const luabind::object &data) {
    return character->createItem(id, number, quality, convert_to_map(data).get());
}

luabind::object getLoot(const Character *character) {
    lua_State *_luaState = World::get()->getCurrentScript()->getLuaState();
    luabind::object lootTable = luabind::newtable(_luaState);

    try {
        const auto &loot = character->getLoot();

        for (const auto &category: loot) {
            const auto categoryId = category.first;
            const auto &categoryItems = category.second;

            luabind::object lootTableCategory = luabind::newtable(_luaState);

            for (const auto &item: categoryItems) {
                const auto lootId = item.first;
                const auto &lootItem = item.second;
                //auto &lootTableItem = lootTable[categoryId][lootId];

                //probability, itemId, minAmount, maxAmount, minQuality, maxQuality, minDurability, maxDurability
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
                
                for (const auto &data: lootItem.data) {
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

int container_count_item1(Container *container, Item::id_type id) {
    return container->countItem(id);
}

int container_count_item2(Container *container, Item::id_type id, const luabind::object &data) {
    return container->countItem(id, convert_to_map(data).get());
}

int container_erase_item1(Container *container, Item::id_type id, Item::number_type number) {
    return container->eraseItem(id, number);
}

int container_erase_item2(Container *container, Item::id_type id, Item::number_type number, const luabind::object &data) {
    return container->eraseItem(id, number, convert_to_map(data).get());
}

Field *world_fieldAt(World *world, const position &pos) {
    try {
        return &world->fieldAt(pos);
    } catch (FieldNotFound &) {
        return nullptr;
    }
}

ScriptItem world_createFromId(World *world, TYPE_OF_ITEM_ID id, unsigned short int count, position pos, bool always, int quali, const luabind::object &data) {
    return world->createFromId(id, count, pos, always, quali, convert_to_map(data).get());
}

void log_lua(const std::string &message) {
    Logger::info(LogFacility::Script) << message << Log::end;
}

luabind::object character_getItemList(const Character *character, TYPE_OF_ITEM_ID id) {
    const auto &content = character->getItemList(id);
    lua_State *_luaState = World::get()->getCurrentScript()->getLuaState();
    luabind::object list = luabind::newtable(_luaState);

    int index = 1;

    for (auto item : content) {
        list[index++] = item;
    }

    return list;
}

luabind::object waypointlist_getWaypoints(const WaypointList *wpl) {
    lua_State *luaState = World::get()->getCurrentScript()->getLuaState();
    luabind::object list = luabind::newtable(luaState);

    int index = 1;

    const auto &waypoints = wpl->getWaypoints();

    for (const auto& waypoint : waypoints) {
        list[index++] = waypoint;
    }

    return list;

}

void waypointlist_addFromList(WaypointList *wpl, const luabind::object &list) {
    if (list.is_valid()) {
        if (luabind::type(list) == LUA_TTABLE) {
            for (luabind::iterator it(list), end; it != end; ++it) {
                try {
                    position pos = luabind::object_cast<position>(*it);
                    wpl->addWaypoint(pos);
                } catch (luabind::cast_failed &e) {
                    const std::string script = World::get()->getCurrentScript()->getFileName();
                    Logger::error(LogFacility::Script) << "Invalid type in parameter list of WaypointList:addFromList in " << script << ": " << "Expected type position" << Log::end;
                }
            }
        }
    }
}

luabind::object world_LuaLoS(const World *world, const position &startingpos, const position &endingpos) {
    lua_State *luaState = world->getCurrentScript()->getLuaState();
    luabind::object list = luabind::newtable(luaState);
    int index = 1;
    auto objects = world->LoS(startingpos, endingpos);

    for (auto &blocker: objects) {
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

luabind::object world_getPlayersOnline(const World *world) {
    lua_State *luaState = world->getCurrentScript()->getLuaState();
    luabind::object list = luabind::newtable(luaState);
    int index = 1;

    const auto &players = world->getPlayersOnline();

    for (auto player : players) {
        list[index] = character_ptr(player);
        index++;
    }

    return list;
}

template<typename Container>
luabind::object convert_to_fuselist(const Container &container) {
    lua_State *luaState = World::get()->getCurrentScript()->getLuaState();
    luabind::object list = luabind::newtable(luaState);
    int index = 1;

    for (auto item : container) {
        list[index] = character_ptr(item);
        index++;
    }

    return list;
}

luabind::object world_getNPCS(const World *world) {
    return convert_to_fuselist(world->getNPCS());
}

luabind::object world_getCharactersInRangeOf(const World *world, const position &posi, uint8_t range) {
    return convert_to_fuselist(world->getCharactersInRangeOf(posi, range));
}

luabind::object world_getPlayersInRangeOf(const World *world, const position &posi, uint8_t range) {
    return convert_to_fuselist(world->getPlayersInRangeOf(posi, range));
}

luabind::object world_getMonstersInRangeOf(const World *world, const position &posi, uint8_t range) {
    return convert_to_fuselist(world->getMonstersInRangeOf(posi, range));
}

luabind::object world_getNPCSInRangeOf(const World *world, const position &posi, uint8_t range) {
    return convert_to_fuselist(world->getNPCSInRangeOf(posi, range));
}
