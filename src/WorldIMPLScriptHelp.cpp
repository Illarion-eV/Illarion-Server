//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.


#include "World.hpp"

#include "Player.hpp"
#include "NPC.hpp"
#include "Monster.hpp"
#include "Item.hpp"
#include "Field.hpp"
#include "Map.hpp"
#include "MonitoringClients.hpp"
#include "Logger.hpp"
#include "character_ptr.hpp"

#include "data/Data.hpp"
#include "data/NaturalArmorTable.hpp"

#include "script/LuaNPCScript.hpp"

#include "netinterface/protocol/ServerCommands.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"

bool World::deleteNPC(unsigned int npcid) {
    /*
    int posx,posy,posz;
    Field* tempf; //alte NPC's l?chen
    NPCVECTOR::iterator npcIterator;
    for ( npcIterator = Npc.begin(); npcIterator < Npc.end(); ++npcIterator ) {
        if (( *npcIterator )->id == npcid) {
            if ( GetPToCFieldAt( tempf, ( *npcIterator )->pos.x, ( *npcIterator )->pos.y, ( *npcIterator )->pos.z ) ) {
                //tempf->SetNPCOnField( false );
                tempf->removeChar();
            }

            sendRemoveCharToVisiblePlayers( ( *npcIterator )->id, ( *npcIterator )->pos );
            delete( *npcIterator );
            Npc.erase( npcIterator );
            return true;
        }
    }
    return false;*/
    LostNpcs.push_back(npcid);
    return true;
}

bool World::createDynamicNPC(const std::string &name, TYPE_OF_RACE_ID type, const position &pos, /*CCharacter::face_to dir,*/ Character::sex_type sex, const std::string &scriptname) {
    try {

        try {
            NPC *newNPC = new NPC(DYNNPC_BASE, name, type, pos, (Character::face_to)4/*dir*/, false, sex, {});

            // add npc to npc list
            Npc.insert(newNPC);

            // set field to occupied
            Field *tempf;

            if (GetPToCFieldAt(tempf, pos)) {
                tempf->setChar();
            }

            try {
                // try to load the script
                std::shared_ptr<LuaNPCScript> script(new LuaNPCScript(scriptname, newNPC));
                newNPC->setScript(script);
            } catch (ScriptException &e) {
                Logger::error(LogFacility::Script) << "World::createDynamicNPC: Error while loading dynamic NPC script: " << scriptname << ": " << e.what() << Log::end;
            }
        } catch (NoSpace &s) {
            Logger::error(LogFacility::Script) << "World::createDynamicNPC: No space available for dynamic NPC: " << name << ": " << s.what() << Log::end;
        }

        return true;
    } catch (...) {
        std::cerr << "World::createDynamicNPC: Unknown error while loading dynamic NPC: " << name << std::endl;
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

std::vector<Character *> World::getCharactersInRangeOf(const position &pos, uint8_t range) const {
    std::vector<Character *> list;

    std::vector < Player * > tempP = Players.findAllCharactersInRangeOf(pos, range);
    list.insert(list.end(), tempP.begin(), tempP.end());

    std::vector < Monster * > tempM = Monsters.findAllCharactersInRangeOf(pos, range);
    list.insert(list.end(), tempM.begin(), tempM.end());

    std::vector < NPC * > tempN = Npc.findAllCharactersInRangeOf(pos, range);
    list.insert(list.end(), tempN.begin(), tempN.end());

    return list;
}

std::vector<Player *> World::getPlayersInRangeOf(const position &pos, uint8_t range) const {
    return Players.findAllCharactersInRangeOf(pos, range);
}

std::vector<Monster *> World::getMonstersInRangeOf(const position &pos, uint8_t range) const {
    return Monsters.findAllCharactersInRangeOf(pos, range);
}

std::vector<NPC *> World::getNPCSInRangeOf(const position &pos, uint8_t range) const {
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
        ServerCommandPointer cmd = std::make_shared<LookAtMapItemTC>(item.pos, lookAt);
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

void World::changeQualityOfItemAt(const position &pos, short int amount) {
    Field *field;

    if (GetPToCFieldAt(field, pos)) {
        if (field->changeQualityOfTopItem(amount)) {
            sendRemoveItemFromMapToAllVisibleCharacters(pos);
        }
    }
}

bool World::changeItem(ScriptItem item) {
    if (item.type == ScriptItem::it_inventory || item.type == ScriptItem::it_belt) {
        item.owner->characterItems[ item.itempos ] = (Item)item;

        //Wenn character ein Spieler ist ein update schicken
        if (item.owner->getType() == Character::player) {
            dynamic_cast<Player *>(item.owner)->sendCharacterItemAtPos(item.itempos);
        }

        item.owner->updateAppearanceForAll(true);
        return true;
    } else if (item.type == ScriptItem::it_field) {
        Field *field;
        Item dummy;

        if (GetPToCFieldAt(field, item.pos)) {
            Item it;

            if (field->TakeTopItem(it)) {
                field->PutTopItem(static_cast<Item>(item));

                if (item.getId() != it.getId() || it.getNumber() != item.getNumber()) {
                    sendSwapItemOnMapToAllVisibleCharacter(it.getId(), item.pos, item);
                }
            }

            return true;
        } else {
            return false;
        }
    } else if (item.type == ScriptItem::it_container) {
        if (item.inside) {
            item.inside->changeItem(item);
            sendChangesOfContainerContentsIM(item.inside);
            return true;
        }
    }

    return false;
}

std::string World::getItemName(TYPE_OF_ITEM_ID itemid, uint8_t language) {
    if (language == 0) {
        return Data::ItemNames[itemid].German;
    } else {
        return Data::ItemNames[itemid].English;
    }
}


CommonStruct World::getItemStats(ScriptItem item) {
    const auto &data = Data::CommonItems[item.getId()];
    return data;
}

CommonStruct World::getItemStatsFromId(TYPE_OF_ITEM_ID id) {
    const auto &data = Data::CommonItems[id];
    return data;
}

bool World::isCharacterOnField(const position &pos) {
    if (findCharacterOnField(pos)) {
        return true;
    } else {
        return false;
    }
}

character_ptr World::getCharacterOnField(const position &pos) {
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
        Field *field;
        Item dummy;

        if (GetPToCFieldAt(field, item.pos)) {
            bool erased=false;
            field->increaseTopItem(-amount, erased);

            if (erased) {
                sendRemoveItemFromMapToAllVisibleCharacters(item.pos);
            }

            return true;
        } else {
            logMissingField("erase", item.pos);
            return false;
        }
    } else if (item.type == ScriptItem::it_container) {
        if (item.inside) {
            item.inside->increaseAtPos(item.itempos, -amount);
            sendChangesOfContainerContentsIM(item.inside);
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
    }
    //Item befindet sich auf einen Feld am Boden liegend.
    else if (item.type == ScriptItem::it_field) {
        Field *field;

        if (GetPToCFieldAt(field, item.pos)) {
            bool erased=false;
            field->increaseTopItem(count, erased);

            if (erased) {
                sendRemoveItemFromMapToAllVisibleCharacters(item.pos);
            }

            return true;
        } else {
            logMissingField("increase", item.pos);
            return false;
        }
    }

    else if (item.type == ScriptItem::it_container) {
        if (item.inside) {
            item.inside->increaseAtPos(item.itempos, count);
            sendChangesOfContainerContentsIM(item.inside);
            return true;
        } else {
            return false;
        }
    }

    return false;
}

bool World::swap(ScriptItem item, TYPE_OF_ITEM_ID newitem, unsigned short int newQuality) {
    if (item.type == ScriptItem::it_inventory || item.type == ScriptItem::it_belt) {
        item.owner->swapAtPos(item.itempos, newitem, newQuality);
        return true;
    }
    //Item befindet sich auf einen Feld am Boden liegend.
    else if (item.type == ScriptItem::it_field) {
        Field *field;

        if (GetPToCFieldAt(field, item.pos)) {
            Item it;

            if (field->ViewTopItem(it)) {
                bool ok = field->swapTopItem(newitem, newQuality);

                if (ok) {
                    Item dummy;
                    dummy.setId(newitem);
                    dummy.setNumber(it.getNumber());

                    if (it.getId() != newitem) {
                        sendSwapItemOnMapToAllVisibleCharacter(it.getId(), item.pos, dummy);
                    }
                } else {
                    Logger::error(LogFacility::Script) << "World::swap: Swapping item on Field " << item.pos << " failed!" << Log::end;
                    return false;
                }
            }
        } else {
            logMissingField("swap", item.pos);
            return false;
        }
    }

    else if (item.type == ScriptItem::it_container) {
        if (item.inside) {
            item.inside->swapAtPos(item.itempos, newitem, newQuality);
            sendChangesOfContainerContentsIM(item.inside);
            return true;
        } else {
            return false;
        }
    }


    return false;
}

ScriptItem World::createFromId(TYPE_OF_ITEM_ID id, unsigned short int count, const position &pos, bool always, int quality, script_data_exchangemap const *data) {
    Field *field;
    ScriptItem sItem;

    if (GetPToCFieldAt(field, pos)) {
        const auto &com = Data::CommonItems[id];

        if (com.isValid()) {
            g_item.setId(id);
            g_item.setNumber(count);
            g_item.setWear(com.AgeingSpeed);
            g_item.setQuality(quality);
            g_item.setData(data);
            g_cont = nullptr;
            sItem = g_item;
            sItem.pos = pos;
            sItem.type = ScriptItem::it_field;
            sItem.itempos = 255;
            sItem.owner = nullptr;

            if (always) {
                putItemAlwaysOnMap(nullptr, pos);
            } else {
                putItemOnMap(nullptr, pos);
            }

            return sItem;
        } else {
            Logger::error(LogFacility::Script) << "World::createFromId: Item " << id << " was not found in CommonItems!" << Log::end;
            return sItem;
        }
    } else {
        logMissingField("createFromId", pos);
        return sItem;
    }

    return sItem;

}

bool World::createFromItem(ScriptItem item, const position &pos, bool always) {
    Field *field;

    if (GetPToCFieldAt(field, pos)) {
        g_item = static_cast<Item>(item);
        g_cont = nullptr;

        if (always) {
            putItemAlwaysOnMap(nullptr, pos);
        } else {
            putItemOnMap(nullptr, pos);
        }

        return true;
    } else {
        logMissingField("createFromItem", pos);
        return false;
    }

    return false;
}

character_ptr World::createMonster(unsigned short id, const position &pos, short movepoints) {
    Field *field;

    if (GetPToCFieldAt(field, pos)) {
        try {
            Monster *newMonster = new Monster(id, pos);
#ifdef LUASCRIPT_DEBUG
            std::cout<<"Erschaffe neues Monster: " << newMonster->name << " an Position (x,y,z) " << pos.x << " " << pos.y << " " << pos.z << std::endl;
#endif
            newMonster->setActionPoints(movepoints);
            newMonsters.push_back(newMonster);
            field->setChar();
            sendCharacterMoveToAllVisiblePlayers(newMonster, NORMALMOVE, 4);
            return character_ptr(newMonster);

        } catch (Monster::unknownIDException &) {
            Logger::error(LogFacility::Script) << "World::createMonster: Failed to create monster with unknown id " << id << "!" << Log::end;
            return character_ptr();
        }
    } else {
        logMissingField("createMonster", pos);
        return character_ptr();
    }

    return character_ptr();
}

void World::gfx(unsigned short int gfxid, const position &pos) {
    std::vector < Player * > temp = Players.findAllCharactersInScreen(pos);
    std::vector < Player * > ::iterator titerator;

    for (titerator = temp.begin(); titerator < temp.end(); ++titerator) {
        ServerCommandPointer cmd = std::make_shared<GraphicEffectTC>(pos, gfxid);
        (*titerator)->Connection->addCommand(cmd);
    }
}

void World::makeSound(unsigned short int soundid, const position &pos) {
    std::vector < Player * > temp = Players.findAllCharactersInScreen(pos);
    std::vector < Player * > ::iterator titerator;

    for (titerator = temp.begin(); titerator < temp.end(); ++titerator) {
        ServerCommandPointer cmd = std::make_shared<SoundTC>(pos, soundid);
        (*titerator)->Connection->addCommand(cmd);
    }
}

bool World::isItemOnField(const position &pos) {
    Field *field;

    if (GetPToCFieldAt(field, pos)) {
        Item dummy;
        return field->ViewTopItem(dummy);
    } else {
        logMissingField("isItemOnField", pos);
    }

    return false;
}

ScriptItem World::getItemOnField(const position &pos) {
    Field *field;
    ScriptItem item;

    if (GetPToCFieldAt(field, pos)) {
        Item It;

        if (field->ViewTopItem(It)) {
            item = It;
            item.pos = pos;
            item.type = ScriptItem::it_field;
            return item;
        }
    } else {
        logMissingField("getItemOnField", pos);
    }

    return item;
}

void World::changeTile(short int tileid, const position &pos) {
    Field *field;

    if (GetPToCFieldAt(field, pos)) {
        field->setTileId(tileid);
        field->updateFlags();
    }
}


void World::sendMapUpdate(const position &pos, uint8_t range) {
    std::vector<Player *> temp;
    std::vector<Player *>::iterator pIterator;
    temp=Players.findAllCharactersInRangeOf(pos, range);

    for (pIterator = temp.begin(); pIterator != temp.end(); ++pIterator) {
        (*pIterator)->sendFullMap();
        sendAllVisibleCharactersToPlayer((*pIterator), true);
    }
}

bool World::createSavedArea(uint16_t tileid, const position &pos, uint16_t height, uint16_t width) {
    WorldMap::map_t dummy;

    for (time_t akt_x = pos.x; akt_x < pos.x+width; ++akt_x) {
        for (time_t akt_y = pos.y; akt_y < pos.y+height; ++akt_y) {
            const position testPos(akt_x, akt_y, pos.z);

            if (maps.findMapForPos(testPos, dummy)) {
                std::cerr<<"World::createSavedArea: Aborted map insertion, map for field at "<< testPos << " found!"<<std::endl;
                return false;
            }
        }
    }

    auto tempmap = std::make_shared<Map>(width,height);
    tempmap->Init(pos.x, pos.y, pos.z);

    Field *tempf;

    for (int _x=0; _x<width; ++_x)
        for (int _y=0; _y<height; ++_y) {
            if (tempmap->GetPToCFieldAt(tempf, _x+pos.x, _y+pos.y)) {
                tempf->setTileId(tileid);
                tempf->updateFlags();
            } else {
                std::cerr << "World::createSavedArea: For map inserted at " << pos << " no Field was found for offset (" << _x << ", " << _y << ")!" << std::endl;
            }

        }

    maps.InsertMap(tempmap);
    std::cout<<" Map Created by createSavedArea command at " << pos <<" height: "<<height<<" width: "<<width<<" standard tile: "<<tileid<<"!"<<std::endl;
    return true;
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

