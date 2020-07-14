//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.

#include "Item.hpp"
#include "Logger.hpp"
#include "MonitoringClients.hpp"
#include "Monster.hpp"
#include "NPC.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "character_ptr.hpp"
#include "data/Data.hpp"
#include "data/NaturalArmorTable.hpp"
#include "map/Field.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "script/LuaNPCScript.hpp"

auto World::deleteNPC(unsigned int npcid) -> bool {
    LostNpcs.push_back(npcid);
    return true;
}

auto World::createDynamicNPC(const std::string &name, TYPE_OF_RACE_ID type, const position &pos,
                             /*CCharacter::face_to dir,*/ Character::sex_type sex, const std::string &scriptname)
        -> bool {
    try {
        try {
            NPC *newNPC = new NPC(DYNNPC_BASE, name, type, pos, (Character::face_to)4 /*dir*/, false, sex, {});

            // add npc to npc list
            Npc.insert(newNPC);

            try {
                // try to load the script
                auto script = std::make_shared<LuaNPCScript>(scriptname, newNPC);
                newNPC->setScript(script);
            } catch (ScriptException &e) {
                Logger::error(LogFacility::Script)
                        << "World::createDynamicNPC: Error while loading dynamic NPC script: " << scriptname << ": "
                        << e.what() << Log::end;
            }
        } catch (FieldNotFound &) {
            Logger::error(LogFacility::Script)
                    << "World::createDynamicNPC: No space available for dynamic NPC: " << name << " near " << pos
                    << Log::end;
        }

        return true;
    } catch (...) {
        Logger::error(LogFacility::Script)
                << "World::createDynamicNPC: Unknown error while loading dynamic NPC: " << name << Log::end;
        return false;
    }
}

auto World::getPlayersOnline() const -> std::vector<Player *> {
    std::vector<Player *> list;

    Players.for_each([&list](Player *player) { list.push_back(player); });

    return list;
}

auto World::getNPCS() const -> std::vector<NPC *> {
    std::vector<NPC *> list;

    Npc.for_each([&list](NPC *npc) { list.push_back(npc); });

    return list;
}

auto World::getCharactersInRangeOf(const position &pos, uint8_t radius) const -> std::vector<Character *> {
    std::vector<Character *> list;
    Range range;
    range.radius = radius;

    std::vector<Player *> tempP = Players.findAllCharactersInRangeOf(pos, range);
    list.insert(list.end(), tempP.begin(), tempP.end());

    std::vector<Monster *> tempM = Monsters.findAllCharactersInRangeOf(pos, range);
    list.insert(list.end(), tempM.begin(), tempM.end());

    std::vector<NPC *> tempN = Npc.findAllCharactersInRangeOf(pos, range);
    list.insert(list.end(), tempN.begin(), tempN.end());

    return list;
}

auto World::getPlayersInRangeOf(const position &pos, uint8_t radius) const -> std::vector<Player *> {
    Range range;
    range.radius = radius;
    return Players.findAllCharactersInRangeOf(pos, range);
}

auto World::getMonstersInRangeOf(const position &pos, uint8_t radius) const -> std::vector<Monster *> {
    Range range;
    range.radius = radius;
    return Monsters.findAllCharactersInRangeOf(pos, range);
}

auto World::getNPCSInRangeOf(const position &pos, uint8_t radius) const -> std::vector<NPC *> {
    Range range;
    range.radius = radius;
    return Npc.findAllCharactersInRangeOf(pos, range);
}

void World::itemInform(Character *user, const ScriptItem &item, const ItemLookAt &lookAt) {
    if (user->getType() != Character::player) {
        return;
    }

    auto *cp = dynamic_cast<Player *>(user);

    if (item.type == ScriptItem::it_container) {
        if (item.inside != nullptr) {
            try {
                uint8_t showcase = cp->getShowcaseId(item.inside);
                ServerCommandPointer cmd = std::make_shared<LookAtShowCaseItemTC>(showcase, item.itempos, lookAt);
                cp->Connection->addCommand(cmd);
            } catch (std::logic_error &) {
            }
        }
    } else if (item.type == ScriptItem::it_inventory || item.type == ScriptItem::it_belt) {
        ServerCommandPointer cmd = std::make_shared<LookAtInventoryItemTC>(item.itempos, lookAt);
        cp->Connection->addCommand(cmd);
    } else if (item.type == ScriptItem::it_field) {
        ServerCommandPointer cmd = std::make_shared<LookAtMapItemTC>(item.pos, item.itempos, lookAt);
        cp->Connection->addCommand(cmd);
    }
}

void World::changeQuality(ScriptItem item, int amount) {
    int tmpQuality = ((amount + item.getDurability()) <= Item::maximumDurability)
                             ? (amount + item.getQuality())
                             : (item.getQuality() - item.getDurability() + Item::maximumDurability);

    item.setQuality(tmpQuality);
    changeItem(item);
}

auto World::changeItem(ScriptItem item) -> bool {
    if (item.type == ScriptItem::it_inventory || item.type == ScriptItem::it_belt) {
        item.owner->items.at(item.itempos) = item.cloneItem();

        if (item.owner->getType() == Character::player) {
            dynamic_cast<Player *>(item.owner)->sendCharacterItemAtPos(item.itempos);
        }

        item.owner->updateAppearanceForAll(true);
        return true;
    }
    if (item.type == ScriptItem::it_field) {
        try {
            map::Field &field = fieldAt(item.pos);

            Item it;

            if (field.takeItemFromStack(it)) {
                field.addItemOnStack(item.cloneItem());

                if (item.getId() != it.getId() || it.getNumber() != item.getNumber()) {
                    sendSwapItemOnMapToAllVisibleCharacter(it.getId(), item.pos, item);
                }
            }

            return true;

        } catch (FieldNotFound &) {
            return false;
        }

    } else if (item.type == ScriptItem::it_container) {
        if (item.inside != nullptr) {
            item.inside->changeItem(item);

            sendContainerSlotChange(item.inside, item.itempos);

            return true;
        }
    }

    return false;
}

auto World::getItemName(TYPE_OF_ITEM_ID itemid, uint8_t language) const -> std::string {
    if (language == 0) {
        return Data::items()[itemid].German;
    }
    return Data::items()[itemid].English;
}

auto World::getItemStats(const ScriptItem &item) const -> ItemStruct {
    const auto &data = Data::items()[item.getId()];
    return data;
}

auto World::getItemStatsFromId(TYPE_OF_ITEM_ID id) const -> ItemStruct {
    const auto &data = Data::items()[id];
    return data;
}

auto World::isCharacterOnField(const position &pos) const -> bool { return findCharacterOnField(pos) != nullptr; }

auto World::getCharacterOnField(const position &pos) const -> character_ptr {
    return character_ptr(findCharacterOnField(pos));
}

auto World::erase(ScriptItem item, int amount) -> bool {
    if (amount > item.getNumber()) {
        amount = item.getNumber();
    }

    if (item.type == ScriptItem::it_inventory || item.type == ScriptItem::it_belt) {
        if (item.itempos == RIGHT_TOOL && (item.owner->GetItemAt(LEFT_TOOL)).getId() == BLOCKEDITEM) {
            item.owner->increaseAtPos(LEFT_TOOL, -MAXITEMS);
        } else if (item.itempos == LEFT_TOOL && (item.owner->GetItemAt(RIGHT_TOOL)).getId() == BLOCKEDITEM) {
            item.owner->increaseAtPos(RIGHT_TOOL, -MAXITEMS);
        }

        item.owner->increaseAtPos(item.itempos, -amount);
        return true;
    }
    if (item.type == ScriptItem::it_field) {
        try {
            map::Field &field = fieldAt(item.pos);

            bool erased = false;

            field.increaseItemOnStack(-amount, erased);

            if (erased) {
                sendRemoveItemFromMapToAllVisibleCharacters(item.pos);

            } else {
                sendSwapItemOnMapToAllVisibleCharacter(item.getId(), item.pos, item);
            }

            return true;

        } catch (FieldNotFound &) {
            logMissingField("erase", item.pos);

            return false;
        }

    } else if (item.type == ScriptItem::it_container) {
        if (item.inside != nullptr) {
            item.inside->increaseAtPos(item.itempos, -amount);

            sendContainerSlotChange(item.inside, item.itempos);

            return true;
        }
        return false;
    }

    return false;
}

auto World::increase(ScriptItem item, int count) -> bool {
    if (item.type == ScriptItem::it_inventory || item.type == ScriptItem::it_belt) {
        item.owner->increaseAtPos(item.itempos, count);
        return true;
    }
    if (item.type == ScriptItem::it_field) {
        try {
            bool erased = false;

            fieldAt(item.pos).increaseItemOnStack(count, erased);

            if (erased) {
                sendRemoveItemFromMapToAllVisibleCharacters(item.pos);
            }

            return true;

        } catch (FieldNotFound &) {
            logMissingField("increase", item.pos);

            return false;
        }

    } else if (item.type == ScriptItem::it_container) {
        if (item.inside != nullptr) {
            item.inside->increaseAtPos(item.itempos, count);

            sendContainerSlotChange(item.inside, item.itempos);

            return true;
        }
        return false;
    }

    return false;
}

auto World::swap(ScriptItem item, TYPE_OF_ITEM_ID newItem, int newQuality) -> bool {
    if (item.type == ScriptItem::it_inventory || item.type == ScriptItem::it_belt) {
        item.owner->swapAtPos(item.itempos, newItem, newQuality);
        return true;
    }
    if (item.type == ScriptItem::it_field) {
        try {
            map::Field &field = fieldAt(item.pos);

            Item it;

            if (field.viewItemOnStack(it)) {
                if (field.swapItemOnStack(newItem, newQuality)) {
                    Item dummy;

                    dummy.setId(newItem);

                    dummy.setNumber(it.getNumber());

                    if (it.getId() != newItem) {
                        sendSwapItemOnMapToAllVisibleCharacter(it.getId(), item.pos, dummy);
                    }

                } else {
                    Logger::error(LogFacility::Script)

                            << "World::swap: Swapping item on Field " << item.pos << " failed!" << Log::end;

                    return false;
                }
            }

        } catch (FieldNotFound &) {
            logMissingField("swap", item.pos);

            return false;
        }

    } else if (item.type == ScriptItem::it_container) {
        if (item.inside != nullptr) {
            item.inside->swapAtPos(item.itempos, newItem, newQuality);

            sendContainerSlotChange(item.inside, item.itempos);

            return true;
        }
        return false;
    }

    return false;
}

auto World::createFromId(TYPE_OF_ITEM_ID id, int count, const position &pos, bool always, int quality,
                         script_data_exchangemap const *data) -> ScriptItem {
    ScriptItem sItem;

    const auto &com = Data::items()[id];

    if (com.isValid()) {
        g_item.setId(id);
        g_item.setNumber(count);
        g_item.setWear(com.AgeingSpeed);
        g_item.setQuality(quality);
        g_item.setData(data);
        g_cont = nullptr;
        sItem = static_cast<ScriptItem>(g_item);
        sItem.pos = pos;
        sItem.type = ScriptItem::it_field;
        sItem.itempos = ScriptItem::maxItemPos;
        sItem.owner = nullptr;

        if (always) {
            putItemAlwaysOnMap(nullptr, pos);
        } else {
            putItemOnMap(nullptr, pos);
        }
    } else {
        Logger::error(LogFacility::Script)
                << "World::createFromId: Item " << id << " was not found in items!" << Log::end;
    }

    return sItem;
}

auto World::createFromItem(const ScriptItem &item, const position &pos, bool always) -> bool {
    g_item = item.cloneItem();
    g_cont = nullptr;

    if (always) {
        putItemAlwaysOnMap(nullptr, pos);
    } else {
        putItemOnMap(nullptr, pos);
    }

    return true;
}

auto World::createMonster(unsigned short id, const position &pos, short movepoints) -> character_ptr {
    try {
        map::Field &field = fieldAt(pos);

        try {
            auto *newMonster = new Monster(id, pos);
            newMonster->setActionPoints(movepoints);
            newMonsters.push_back(newMonster);
            field.setChar();
            return character_ptr(newMonster);

        } catch (Monster::unknownIDException &) {
            Logger::error(LogFacility::Script)
                    << "World::createMonster: Failed to create monster with unknown id " << id << "!" << Log::end;
            return character_ptr();
        }
    } catch (FieldNotFound &) {
        logMissingField("createMonster", pos);
        return character_ptr();
    }

    return character_ptr();
}

void World::gfx(unsigned short int gfxid, const position &pos) const {
    for (auto &player : Players.findAllCharactersInScreen(pos)) {
        ServerCommandPointer cmd = std::make_shared<GraphicEffectTC>(pos, gfxid);
        player->Connection->addCommand(cmd);
    }
}

void World::makeSound(unsigned short int soundid, const position &pos) const {
    for (auto &player : Players.findAllCharactersInScreen(pos)) {
        ServerCommandPointer cmd = std::make_shared<SoundTC>(pos, soundid);
        player->Connection->addCommand(cmd);
    }
}

auto World::isItemOnField(const position &pos) -> bool {
    try {
        return fieldAt(pos).itemCount() > 0;
    } catch (FieldNotFound &) {
        logMissingField("isItemOnField", pos);
        return false;
    }
}

auto World::getItemOnField(const position &pos) -> ScriptItem {
    ScriptItem item;

    try {
        map::Field &field = fieldAt(pos);
        Item it;

        if (field.viewItemOnStack(it)) {
            item = static_cast<ScriptItem>(it);
            item.pos = pos;
            item.type = ScriptItem::it_field;
        }
    } catch (FieldNotFound &) {
        logMissingField("getItemOnField", pos);
    }

    return item;
}

void World::changeTile(short int tileid, const position &pos) {
    try {
        map::Field &field = fieldAt(pos);
        field.setTileId(tileid);
    } catch (FieldNotFound &) {
        logMissingField("changeTile", pos);
    }
}

auto World::createSavedArea(uint16_t tile, const position &origin, uint16_t height, uint16_t width) -> bool {
    if (maps.createMap("by createSavedArea", origin, width, height, tile)) {
        Logger::info(LogFacility::World) << "Map created by createSavedArea command at " << origin
                                         << " height: " << height << " width: " << width << " standard tile: " << tile
                                         << "!" << Log::end;
        return true;
    }
    return false;
}

auto World::getArmorStruct(TYPE_OF_ITEM_ID id, ArmorStruct &ret) -> bool {
    // Has to be an own function cant give a pointer of Armor items to the script

    if (Data::armorItems().exists(id)) {
        ret = Data::armorItems()[id];
        return true;
    }
    return false;
}

auto World::getWeaponStruct(TYPE_OF_ITEM_ID id, WeaponStruct &ret) -> bool {
    // Has to be an own function cant give a pointer of Armor items to the script

    if (Data::weaponItems().exists(id)) {
        ret = Data::weaponItems()[id];
        return true;
    }
    return false;
}

auto World::getNaturalArmor(TYPE_OF_RACE_ID id, MonsterArmor &ret) -> bool {
    if (Data::naturalArmors().exists(id)) {
        ret = Data::naturalArmors()[id];
        return true;
    }
    return false;
}

auto World::getMonsterAttack(TYPE_OF_RACE_ID id, AttackBoni &ret) -> bool {
    if (Data::monsterAttacks().exists(id)) {
        ret = Data::monsterAttacks()[id];
        return true;
    }
    return false;
}

void World::sendMonitoringMessage(const std::string &msg, unsigned char id) const {
    // send this Command to all Monitoring Clients
    ServerCommandPointer cmd = std::make_shared<BBMessageTC>(msg, id);
    monitoringClientList->sendCommand(cmd);
}

void World::logMissingField(const std::string &function, const position &field) {
    Logger::error(LogFacility::Script) << "World::" << function << ": Field " << field << " was not found!" << Log::end;
}
