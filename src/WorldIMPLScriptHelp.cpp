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


#include "character_ptr.hpp"
#include "Field.hpp"
#include "Item.hpp"
#include "Logger.hpp"
#include "Map.hpp"
#include "MonitoringClients.hpp"
#include "Monster.hpp"
#include "NPC.hpp"
#include "Player.hpp"
#include "World.hpp"

#include "data/Data.hpp"
#include "data/NaturalArmorTable.hpp"

#include "netinterface/protocol/ServerCommands.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"

#include "script/LuaNPCScript.hpp"

bool World::deleteNPC(unsigned int npcid) {
    LostNpcs.push_back(npcid);
    return true;
}

bool World::createDynamicNPC(const std::string &name, TYPE_OF_RACE_ID type, const position &pos, /*CCharacter::face_to dir,*/ Character::sex_type sex, const std::string &scriptname) {
    try {

        try {
            NPC *newNPC = new NPC(DYNNPC_BASE, name, type, pos, (Character::face_to)4/*dir*/, false, sex, {});

            // add npc to npc list
            Npc.insert(newNPC);

            try {
                // try to load the script
                std::shared_ptr<LuaNPCScript> script(new LuaNPCScript(scriptname, newNPC));
                newNPC->setScript(script);
            } catch (ScriptException &e) {
                Logger::error(LogFacility::Script) << "World::createDynamicNPC: Error while loading dynamic NPC script: " << scriptname << ": " << e.what() << Log::end;
            }
        } catch (FieldNotFound &) {
            Logger::error(LogFacility::Script) << "World::createDynamicNPC: No space available for dynamic NPC: " << name << " near " << pos << Log::end;
        }

        return true;
    } catch (...) {
        Logger::error(LogFacility::Script) << "World::createDynamicNPC: Unknown error while loading dynamic NPC: " << name << Log::end;
        return false;
    }
}


std::vector<Player *> World::getPlayersOnline() const {
    std::vector<Player *> list;

    Players.for_each([&list](Player *player) {
        list.push_back(player);
    });

    return list;
}

std::vector<NPC *> World::getNPCS() const {
    std::vector<NPC *> list;

    Npc.for_each([&list](NPC *npc) {
        list.push_back(npc);
    });

    return list;
}

std::vector<Character *> World::getCharactersInRangeOf(const position &pos, uint8_t radius) const {
    std::vector<Character *> list;
    Range range;
    range.radius = radius;

    std::vector < Player * > tempP = Players.findAllCharactersInRangeOf(pos, range);
    list.insert(list.end(), tempP.begin(), tempP.end());

    std::vector < Monster * > tempM = Monsters.findAllCharactersInRangeOf(pos, range);
    list.insert(list.end(), tempM.begin(), tempM.end());

    std::vector < NPC * > tempN = Npc.findAllCharactersInRangeOf(pos, range);
    list.insert(list.end(), tempN.begin(), tempN.end());

    return list;
}

std::vector<Player *> World::getPlayersInRangeOf(const position &pos, uint8_t radius) const {
    Range range;
    range.radius = radius;
    return Players.findAllCharactersInRangeOf(pos, range);
}

std::vector<Monster *> World::getMonstersInRangeOf(const position &pos, uint8_t radius) const {
    Range range;
    range.radius = radius;
    return Monsters.findAllCharactersInRangeOf(pos, range);
}

std::vector<NPC *> World::getNPCSInRangeOf(const position &pos, uint8_t radius) const {
    Range range;
    range.radius = radius;
    return Npc.findAllCharactersInRangeOf(pos, range);
}

void World::itemInform(Character *user, const ScriptItem &item, const ItemLookAt &lookAt) {
    if (user->getType() != Character::player) {
        return;
    }

    Player *cp = dynamic_cast<Player *>(user);

    if (item.type == ScriptItem::it_container) {
        if (item.inside) {
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


void World::changeQuality(ScriptItem item, short int amount) {
    short int tmpQuality = ((amount+item.getDurability())<100) ? (amount + item.getQuality()) : (item.getQuality() - item.getDurability() + 99);

    if (tmpQuality%100 > 0) {
        item.setQuality(tmpQuality);
        changeItem(item);
    } else {
        erase(item, item.getNumber());
    }
}

bool World::changeItem(ScriptItem item) {
    if (item.type == ScriptItem::it_inventory || item.type == ScriptItem::it_belt) {
        item.owner->items[ item.itempos ] = (Item)item;

        //Wenn character ein Spieler ist ein update schicken
        if (item.owner->getType() == Character::player) {
            dynamic_cast<Player *>(item.owner)->sendCharacterItemAtPos(item.itempos);
        }

        item.owner->updateAppearanceForAll(true);
        return true;
    } else if (item.type == ScriptItem::it_field) {
        try {
            Field &field = fieldAt(item.pos);
            Item it;

            if (field.takeItemFromStack(it)) {
                field.addItemOnStack(static_cast<Item>(item));

                if (item.getId() != it.getId() || it.getNumber() != item.getNumber()) {
                    sendSwapItemOnMapToAllVisibleCharacter(it.getId(), item.pos, item);
                }
            }

            return true;
        } catch (FieldNotFound &) {
            return false;
        }
    } else if (item.type == ScriptItem::it_container) {
        if (item.inside) {
            item.inside->changeItem(item);
            sendContainerSlotChange(item.inside, item.itempos);
            return true;
        }
    }

    return false;
}

std::string World::getItemName(TYPE_OF_ITEM_ID itemid, uint8_t language) {
    if (language == 0) {
        return Data::Items[itemid].German;
    } else {
        return Data::Items[itemid].English;
    }
}


ItemStruct World::getItemStats(const ScriptItem &item) {
    const auto &data = Data::Items[item.getId()];
    return data;
}

ItemStruct World::getItemStatsFromId(TYPE_OF_ITEM_ID id) {
    const auto &data = Data::Items[id];
    return data;
}

bool World::isCharacterOnField(const position &pos) const {
    if (findCharacterOnField(pos)) {
        return true;
    } else {
        return false;
    }
}

character_ptr World::getCharacterOnField(const position &pos) const {
    return character_ptr(findCharacterOnField(pos));
}

bool World::erase(ScriptItem item, int amount) {
    if (amount > item.getNumber()) {
        amount = item.getNumber();
    }

    if (item.type == ScriptItem::it_inventory || item.type == ScriptItem::it_belt) {
        //Wenn Item rechts belegt und links ein Belegt ist [Zweihanditem] das Belegt mit l�schen
        if (item.itempos == RIGHT_TOOL && (item.owner->GetItemAt(LEFT_TOOL)).getId() == BLOCKEDITEM) {
            item.owner->increaseAtPos(LEFT_TOOL, -255);
        } else if (item.itempos == LEFT_TOOL && (item.owner->GetItemAt(RIGHT_TOOL)).getId() == BLOCKEDITEM) {
            item.owner->increaseAtPos(RIGHT_TOOL, -255);
        }

        item.owner->increaseAtPos(item.itempos, -amount);
        return true;
    }
    //Item befindet sich auf einen Feld am Boden liegend.
    else if (item.type == ScriptItem::it_field) {
        try {
            Field &field = fieldAt(item.pos);
            bool erased=false;
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
        if (item.inside) {
            item.inside->increaseAtPos(item.itempos, -amount);
            sendContainerSlotChange(item.inside, item.itempos);
            return true;
        } else {
            return false;
        }
    }

    return false;
}


bool World::increase(ScriptItem item, short int count) {
    if (item.type == ScriptItem::it_inventory || item.type == ScriptItem::it_belt) {
        item.owner->increaseAtPos(item.itempos, count);
        return true;
    } else if (item.type == ScriptItem::it_field) {
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
        if (item.inside) {
            item.inside->increaseAtPos(item.itempos, count);
            sendContainerSlotChange(item.inside, item.itempos);
            return true;
        } else {
            return false;
        }
    }

    return false;
}

bool World::swap(ScriptItem item, TYPE_OF_ITEM_ID newItem, unsigned short int newQuality) {
    if (item.type == ScriptItem::it_inventory || item.type == ScriptItem::it_belt) {
        item.owner->swapAtPos(item.itempos, newItem, newQuality);
        return true;
    } else if (item.type == ScriptItem::it_field) {
        try {
            Field &field = fieldAt(item.pos);
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
                    Logger::error(LogFacility::Script) << "World::swap: Swapping item on Field " << item.pos << " failed!" << Log::end;
                    return false;
                }
            }
        } catch (FieldNotFound &) {
            logMissingField("swap", item.pos);
            return false;
        }
    } else if (item.type == ScriptItem::it_container) {
        if (item.inside) {
            item.inside->swapAtPos(item.itempos, newItem, newQuality);
            sendContainerSlotChange(item.inside, item.itempos);
            return true;
        } else {
            return false;
        }
    }

    return false;
}

ScriptItem World::createFromId(TYPE_OF_ITEM_ID id, unsigned short int count, const position &pos, bool always, int quality, script_data_exchangemap const *data) {
    ScriptItem sItem;

    const auto &com = Data::Items[id];

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
        sItem.itempos = 255;
        sItem.owner = nullptr;

        if (always) {
            putItemAlwaysOnMap(nullptr, pos);
        } else {
            putItemOnMap(nullptr, pos);
        }
    } else {
        Logger::error(LogFacility::Script) << "World::createFromId: Item " << id << " was not found in items!" << Log::end;
    }

    return sItem;
}

bool World::createFromItem(ScriptItem item, const position &pos, bool always) {
    g_item = static_cast<Item>(item);
    g_cont = nullptr;

    if (always) {
        putItemAlwaysOnMap(nullptr, pos);
    } else {
        putItemOnMap(nullptr, pos);
    }

    return true;
}

character_ptr World::createMonster(unsigned short id, const position &pos, short movepoints) {
    try {
        Field &field = fieldAt(pos);
        
        try {
            Monster *newMonster = new Monster(id, pos);
            newMonster->setActionPoints(movepoints);
            newMonsters.push_back(newMonster);
            field.setChar();
            return character_ptr(newMonster);

        } catch (Monster::unknownIDException &) {
            Logger::error(LogFacility::Script) << "World::createMonster: Failed to create monster with unknown id " << id << "!" << Log::end;
            return character_ptr();
        }
    } catch (FieldNotFound &) {
        logMissingField("createMonster", pos);
        return character_ptr();
    }

    return character_ptr();
}

void World::gfx(unsigned short int gfxid, const position &pos) {
    for (auto &player : Players.findAllCharactersInScreen(pos)) {
        ServerCommandPointer cmd = std::make_shared<GraphicEffectTC>(pos, gfxid);
        player->Connection->addCommand(cmd);
    }
}

void World::makeSound(unsigned short int soundid, const position &pos) {
    for (auto &player : Players.findAllCharactersInScreen(pos)) {
        ServerCommandPointer cmd = std::make_shared<SoundTC>(pos, soundid);
        player->Connection->addCommand(cmd);
    }
}

bool World::isItemOnField(const position &pos) {
    try {
        return fieldAt(pos).itemCount() > 0;
    } catch (FieldNotFound &) {
        logMissingField("isItemOnField", pos);
        return false;
    }
}

ScriptItem World::getItemOnField(const position &pos) {
    ScriptItem item;

    try {
        Field &field = fieldAt(pos);
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
        Field &field = fieldAt(pos);
        field.setTileId(tileid);
    } catch (FieldNotFound &) {
        logMissingField("changeTile", pos);
    }
}


void World::sendMapUpdate(const position &pos, uint8_t radius) {
    Range range;
    range.radius = radius;
    auto playersInRange = Players.findAllCharactersInRangeOf(pos, range);

    for (auto &player : playersInRange) {
        player->sendFullMap();
        sendAllVisibleCharactersToPlayer(player, true);
    }
}

bool World::createSavedArea(uint16_t tile, const position &origin,
                            uint16_t height, uint16_t width) {
    if (maps.createMap("by createSavedArea", origin, width, height, tile)) {
        Logger::info(LogFacility::World)
            << "Map created by createSavedArea command at " << origin
            << " height: " << height << " width: " << width
            << " standard tile: " << tile << "!" << Log::end;
        return true;
    } else {
        return false;
    }
}


bool World::getArmorStruct(TYPE_OF_ITEM_ID id, ArmorStruct &ret) {
    //Has to be an own function cant give a pointer of Armor items to the script

    if (Data::ArmorItems.exists(id)) {
        ret = Data::ArmorItems[id];
        return true;
    } else {
        return false;
    }
}

bool World::getWeaponStruct(TYPE_OF_ITEM_ID id, WeaponStruct &ret) {
    //Has to be an own function cant give a pointer of Armor items to the script

    if (Data::WeaponItems.exists(id)) {
        ret = Data::WeaponItems[id];
        return true;
    } else {
        return false;
    }
}

bool World::getNaturalArmor(TYPE_OF_RACE_ID id, MonsterArmor &ret) {

    if (Data::NaturalArmors.exists(id)) {
        ret = Data::NaturalArmors[id];
        return true;
    } else {
        return false;
    }
}

bool World::getMonsterAttack(TYPE_OF_RACE_ID id, AttackBoni &ret) {

    if (Data::MonsterAttacks.exists(id)) {
        ret = Data::MonsterAttacks[id];
        return true;
    } else {
        return false;
    }
}

void World::sendMonitoringMessage(const std::string &msg, unsigned char id) {
    //send this Command to all Monitoring Clients
    ServerCommandPointer cmd = std::make_shared<BBMessageTC>(msg, id);
    monitoringClientList->sendCommand(cmd);
}

void World::logMissingField(const std::string &function, const position &field) {
    Logger::error(LogFacility::Script) << "World::" << function << ": Field " << field << " was not found!" << Log::end;
}

