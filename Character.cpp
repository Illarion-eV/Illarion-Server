/*
 *  illarionserver - server for the game Illarion
 *  Copyright 2011 Illarion e.V.
 *
 *  This file is part of illarionserver.
 *
 *  illarionserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  illarionserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Character.hpp"
#include "tuningConstants.hpp"
#include "Random.hpp"
#include "data/ContainerObjectTable.hpp"
#include "data/CommonObjectTable.hpp"
#include "data/WeaponObjectTable.hpp"
#include "Container.hpp"
#include "data/ArmorObjectTable.hpp"
#include "World.hpp"
#include "data/TilesTable.hpp"
#include "script/LuaWeaponScript.hpp"
#include "Logger.hpp"
#include "WaypointList.hpp"
#include <map>
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include <boost/assign/list_of.hpp>
#include "script/LuaLearnScript.hpp"
#include "script/LuaPlayerDeathScript.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "fuse_ptr.hpp"
#include "a_star.hpp"

#define MAJOR_SKILL_GAP 100
#define USE_LUA_FIGTHING

std::ofstream talkfile;

extern ContainerObjectTable *ContainerItems;
extern CommonObjectTable *CommonItems;
extern WeaponObjectTable *WeaponItems;
extern SkillTable *Skills;
extern TilesTable *Tiles;
extern boost::shared_ptr<LuaLearnScript>learnScript;
extern boost::shared_ptr<LuaPlayerDeathScript>playerDeathScript;

boost::unordered_map<std::string, Character::attributeIndex> Character::attributeMap = boost::assign::map_list_of
        ("strength", strength)
        ("dexterity", dexterity)
        ("constitution", constitution)
        ("agility", agility)
        ("intelligence", intelligence)
        ("perception", perception)
        ("willpower", willpower)
        ("essence", essence)
        ("hitpoints", hitpoints)
        ("mana", mana)
        ("foodlevel", foodlevel)
        ("sex", sex)
        ("age", age)
        ("weight", weight)
        ("height", height)
        ("attitude", attitude)
        ("luck", luck);

boost::unordered_map<Character::attributeIndex, std::string> Character::attributeStringMap = boost::assign::map_list_of
        (strength, "strength")
        (dexterity, "dexterity")
        (constitution, "constitution")
        (agility, "agility")
        (intelligence, "intelligence")
        (perception, "perception")
        (willpower, "willpower")
        (essence, "essence")
        (hitpoints, "hitpoints")
        (mana, "mana")
        (foodlevel, "foodlevel")
        (sex, "sex")
        (age, "age")
        (weight, "weight")
        (height, "height")
        (attitude, "attitude")
        (luck, "luck");

position Character::getFrontalPosition() {
    position front = pos;

    switch (faceto) {
    case north:
        --front.y;
        break;
    case northeast:
        --front.y;
        ++front.x;
        break;
    case east:
        ++front.x;
        break;
    case southeast:
        ++front.y;
        ++front.x;
        break;
    case south:
        ++front.y;
        break;
    case southwest:
        ++front.y;
        --front.x;
        break;
    case west:
        --front.x;
        break;
    case northwest:
        --front.y;
        --front.x;
        break;
    }

    return front;
}

luabind::object Character::getLuaStepList(position goal) {
    lua_State *luaState = World::get()->getCurrentScript()->getLuaState();
    luabind::object list = luabind::newtable(luaState);
    int index = 1;
    std::list<Character::direction> dirs;
    getStepList(goal, dirs);

    for (std::list<Character::direction>::iterator it = dirs.begin(); it != dirs.end(); ++it) {
        list[index++] = (*it);
    }

    return list;
}

bool Character::getStepList(position goal, std::list<Character::direction> &steps) {
    return pathfinding::a_star(pos, goal, steps);
}



bool Character::getNextStepDir(position goal, Character::direction &dir) {
    std::list<Character::direction> steps;

    getStepList(goal, steps);

    if (!steps.empty()) {
        dir = steps.front();
        return true;
    } else {
        return false;
    }
}


Character::Character() : actionPoints(P_MAX_AP),fightPoints(P_MAX_FP),waypoints(new WaypointList(this)),_is_on_route(false),_world(World::get()) {
#ifdef Character_DEBUG
    std::cout << "Character Konstruktor Start" << std::endl;
#endif
    race = human;
    character = player;

    isinvisible=false;
    SetAlive(true);
    attackmode = false;
    poisonvalue = 0;
    mental_capacity = 0;
    _movement = walk;

    hair = 0;
    beard = 0;
    hairred = 255;
    hairgreen = 255;
    hairblue = 255;
    skinred = 255;
    skingreen = 255;
    skinblue = 255;

    activeLanguage=0; //common language
    lastSpokenText="";
    //nrOfers=0;
    informCharacter=false;

    pos.x = 0;
    pos.y = 0;
    pos.z = 0;

    for (int i = 0; i < MAX_BODY_ITEMS + MAX_BELT_SLOTS; ++i) {
        characterItems[ i ].reset();
    }

    attributes[strength] = new Attribute(0, MAXATTRIB);
    attributes[dexterity] = new Attribute(0, MAXATTRIB);
    attributes[constitution] = new Attribute(0, MAXATTRIB);
    attributes[agility] = new Attribute(0, MAXATTRIB);
    attributes[intelligence] = new Attribute(0, MAXATTRIB);
    attributes[perception] = new Attribute(0, MAXATTRIB);
    attributes[willpower] = new Attribute(0, MAXATTRIB);
    attributes[essence] = new Attribute(0, MAXATTRIB);
    attributes[hitpoints] = new Attribute(0, MAXHPS);
    attributes[mana] = new Attribute(0, MAXMANA);
    attributes[foodlevel] = new Attribute(0, MAXFOOD);
    attributes[sex] = new Attribute(0);
    attributes[age] = new Attribute(0);
    attributes[weight] = new Attribute(0);
    attributes[height] = new Attribute(0);
    attributes[attitude] = new Attribute(0);
    attributes[luck] = new Attribute(0);


    faceto = north;
    backPackContents = NULL;
    //depotContents = NULL;

    magic.type = MAGE;

    magic.flags[ MAGE ] = 0x00000000;

    magic.flags[ PRIEST ] = 0x00000000;

    magic.flags[ BARD ] = 0x00000000;

    magic.flags[ DRUID ] = 0x00000000;

    effects = new LongTimeCharacterEffects(this);
#ifdef Character_DEBUG
    std::cout << "Character Konstruktor Ende" << std::endl;
#endif
}

Character::~Character() {
#ifdef Character_DEBUG
    std::cout << "Character Destruktor Start" << std::endl;
#endif
    //blow lua fuse for this char
    fuse_ptr<Character>::blow_fuse(this);

    for (int i = 0; i < Character::ATTRIBUTECOUNT; ++i) {
        delete attributes[i];
        attributes[i] = NULL;
    }

    if (backPackContents != NULL) {
        delete backPackContents;
        backPackContents = NULL;
    }

    std::map<uint32_t,Container *>::reverse_iterator rit;

    for (rit = depotContents.rbegin(); rit != depotContents.rend(); ++rit) {
        delete rit->second;
    }

    if (effects != NULL) {
        delete effects;
        effects = NULL;
    }

    if (waypoints != NULL) {
        waypoints->clear();
        delete waypoints;
        waypoints = NULL;
    }

    /*
    if ( depotContents != NULL ) {
      delete depotContents;
      depotContents = NULL;
    }
    */
#ifdef Character_DEBUG
    std::cout << "Character Destruktor Ende" << std::endl;
#endif
}


int Character::countItem(TYPE_OF_ITEM_ID itemid) {
    int temp = 0;

    for (unsigned char i = 0; i < MAX_BELT_SLOTS + MAX_BODY_ITEMS; ++i) {
        if (characterItems[ i ].getId() == itemid) {
            temp = temp + characterItems[ i ].getNumber();
        }
    }

#ifdef Character_DEBUG
    std::cout << "std::coutItem: am K�per gefunden: " << temp << "\n";
#endif

    if ((characterItems[ BACKPACK ].getId() != 0) && (backPackContents != NULL)) {
        temp = temp + backPackContents->countItem(itemid);
    }

#ifdef Character_DEBUG
    std::cout << "std::coutItem: am K�per + Rucksack gefunden: " << temp << "\n";
#endif
    return temp;
}
int Character::countItemAt(std::string where, TYPE_OF_ITEM_ID itemid) {
    int temp = 0;

    if (where == "all") {
        for (unsigned char i = 0; i < MAX_BELT_SLOTS + MAX_BODY_ITEMS; ++i) {
            if (characterItems[ i ].getId() == itemid) {
                temp = temp + characterItems[ i ].getNumber();
            }
        }

#ifdef Character_DEBUG
        std::cout << "std::coutItem: am K�per gefunden: " << temp << "\n";
#endif


        if ((characterItems[ BACKPACK ].getId() != 0) && (backPackContents != NULL)) {
            temp = temp + backPackContents->countItem(itemid);
        }

#ifdef Character_DEBUG
        std::cout << "std::coutItem: am K�per + Rucksack gefunden: " << temp << "\n";
#endif
        return temp;
    }

    if (where == "belt") {
        for (unsigned char i = MAX_BODY_ITEMS; i < MAX_BODY_ITEMS + MAX_BELT_SLOTS; ++i) {
            if (characterItems[ i ].getId() == itemid) {
                temp = temp + characterItems[ i ].getNumber();
            }
        }

        return temp;
    }

    if (where == "body") {
        for (unsigned char i = 0; i < MAX_BODY_ITEMS; ++i) {
            if (characterItems[ i ].getId() == itemid) {
                temp = temp + characterItems[ i ].getNumber();
            }
        }

        return temp;
    }

    if (where == "backpack") {
        if ((characterItems[ BACKPACK ].getId() != 0) && (backPackContents != NULL)) {
            temp = temp + backPackContents->countItem(itemid);
        }

        return temp;
    }

    return temp;
}

int Character::countItemAt(std::string where, TYPE_OF_ITEM_ID itemid, const luabind::object &data) {
    int temp = 0;

    if (where == "all") {
        for (unsigned char i = 0; i < MAX_BELT_SLOTS + MAX_BODY_ITEMS; ++i) {
            if (characterItems[ i ].getId() == itemid && characterItems[i].hasData(data)) {
                temp = temp + characterItems[ i ].getNumber();
            }
        }

#ifdef Character_DEBUG
        std::cout << "std::coutItem: am K�per gefunden: " << temp << "\n";
#endif


        if ((characterItems[ BACKPACK ].getId() != 0) && (backPackContents != NULL)) {
            temp = temp + backPackContents->countItem(itemid, data);
        }

#ifdef Character_DEBUG
        std::cout << "std::coutItem: am K�per + Rucksack gefunden: " << temp << "\n";
#endif
        return temp;
    }

    if (where == "belt") {
        for (unsigned char i = MAX_BODY_ITEMS; i < MAX_BODY_ITEMS + MAX_BELT_SLOTS; ++i) {
            if (characterItems[ i ].getId() == itemid && characterItems[i].hasData(data)) {
                temp = temp + characterItems[ i ].getNumber();
            }
        }

        return temp;
    }

    if (where == "body") {
        for (unsigned char i = 0; i < MAX_BODY_ITEMS; ++i) {
            if (characterItems[ i ].getId() == itemid && characterItems[i].hasData(data)) {
                temp = temp + characterItems[ i ].getNumber();
            }
        }

        return temp;
    }

    if (where == "backpack") {
        if ((characterItems[ BACKPACK ].getId() != 0) && (backPackContents != NULL)) {
            temp = temp + backPackContents->countItem(itemid , data);
        }

        return temp;
    }

    return temp;
}

ScriptItem Character::GetItemAt(unsigned char itempos) {
    ScriptItem item;

    if (itempos < MAX_BODY_ITEMS + MAX_BELT_SLOTS) {
        item = characterItems[ itempos ];
        item.pos = pos;
        item.itempos = itempos;
        item.owner = this;

        if (itempos < MAX_BODY_ITEMS) {
            item.type = ScriptItem::it_inventory;
        } else {
            item.type = ScriptItem::it_belt;
        }
    }

    return item;
}


int Character::_eraseItem(TYPE_OF_ITEM_ID itemid, int count, const luabind::object &data, bool useData) {
    int temp = count;
#ifdef Character_DEBUG
    std::cout << "try to erase in inventory " << count << " items of type " << itemid << " data " << data << "\n";
#endif

    if ((characterItems[ BACKPACK ].getId() != 0) && (backPackContents != NULL)) {
        temp = backPackContents->_eraseItem(itemid, temp, data, useData);
#ifdef Character_DEBUG
        std::cout << "eraseItem: nach L�chen im Rucksack noch zu l�chen: " << temp << "\n";
#endif

    }

    if (temp > 0) {
        // BACKPACK als Item erstmal auslassen
        for (unsigned char i = MAX_BELT_SLOTS + MAX_BODY_ITEMS - 1; i > 0; --i) {
            if ((characterItems[ i ].getId() == itemid && (!useData || characterItems[ i ].hasData(data))) && (temp > 0)) {
                if (temp >= characterItems[ i ].getNumber()) {
                    temp = temp - characterItems[ i ].getNumber();
                    characterItems[ i ].reset();
                } else {
                    characterItems[ i ].setNumber(characterItems[ i ].getNumber() - temp);
                    temp = 0;
                }
            }
        }

        if (World::get()->getItemStatsFromId(itemid).Brightness > 0) {
            updateAppearanceForAll(true);
        }
    }

#ifdef Character_DEBUG
    std::cout << "eraseItem: am Ende noch zu loeschen: " << temp << "\n";
#endif

    return temp;

}


int Character::eraseItem(TYPE_OF_ITEM_ID itemid, int count) {
    const luabind::object nothing;
    return _eraseItem(itemid, count, nothing, false);
}


int Character::eraseItem(TYPE_OF_ITEM_ID itemid, int count, const luabind::object &data) {
    return _eraseItem(itemid, count, data, true);
}


int Character::createAtPos(unsigned char pos, TYPE_OF_ITEM_ID newid, int count) {
    int temp = count;
    Item it;

    if (weightOK(newid, count, NULL)) {
        CommonStruct cos;

        if (CommonItems->find(newid, cos)) {
#ifdef Character_DEBUG
            std::cout<<"createAtPos: itemid gefunden" << std::endl;
#endif

            if (ContainerItems->find(newid)) {
#ifdef Character_DEBUG
                std::cout << "createAtPos: itemid ist ein Container" << std::endl;
#endif

            } else {
                if (characterItems[ pos ].getId() == 0) {
                    if (temp > cos.MaxStack) {
                        characterItems[ pos ].setId(newid);
                        characterItems[ pos ].setWear(cos.AgeingSpeed);
                        characterItems[ pos ].setNumber(cos.MaxStack);
                        temp -= cos.MaxStack;
                    } else {
                        characterItems[ pos ].setId(newid);
                        characterItems[ pos ].setWear(cos.AgeingSpeed);
                        characterItems[ pos ].setNumber(temp);
                        temp = 0;
                    }

                    if (cos.Brightness > 0) {
                        updateAppearanceForAll(true);
                    }
                }
            }
        }
    }

    return temp;
}


int Character::createItem(Item::id_type id, Item::number_type number, Item::quality_type quality, const luabind::object &data) {
    int temp = number;
    Item it;

    if (weightOK(id, number, NULL)) {
        CommonStruct cos;

        if (CommonItems->find(id, cos)) {
#ifdef Character_DEBUG
            std::cout << "createItem: itemid gefunden" << "\n";
#endif

            if (ContainerItems->find(id)) {
#ifdef Character_DEBUG
                std::cout << "createItem: itemid ist ein container" << "\n";
#endif

                if (characterItems[ BACKPACK ].getId() == 0) {
#ifdef Character_DEBUG
                    std::cout << "createItem: erstelle neuen Rucksack" << "\n";
#endif
                    characterItems[ BACKPACK ].setId(id);
                    characterItems[ BACKPACK ].setWear(cos.AgeingSpeed);
                    characterItems[ BACKPACK ].setQuality(quality);
                    characterItems[ BACKPACK ].setData(data);
                    characterItems[ BACKPACK ].setNumber(1);
                    temp = temp - 1;
                    backPackContents = new Container(id);

                    if (cos.Brightness > 0) {
                        updateAppearanceForAll(true);
                    }
                }

                it.setId(id);
                it.setWear(cos.AgeingSpeed);
                it.setQuality(quality);
                it.setNumber(1);
                it.setData(data);

                for (int i = temp; i > 0; i--) {
#ifdef Character_DEBUG
                    std::cout << "createItem: erstelle neuen container im Rucksack" << std::endl;
#endif

                    if (!backPackContents->InsertContainer(it, new Container(it.getId()))) {
                        i = 0;
                    } else {
                        temp = temp - 1;
                    }
                }
            } else {
#ifdef Character_DEBUG
                std::cout << "createItem: normales Item" << std::endl;
#endif
                int old_temp = temp;

                if (cos.MaxStack > 1) {
                    for (unsigned char i = MAX_BODY_ITEMS; i < MAX_BELT_SLOTS + MAX_BODY_ITEMS && temp > 0; ++i) {
                        if ((characterItems[ i ].getId() == id) && (characterItems[ i ].equalData(data))) {
                            int itemsToCreate = temp;
                            temp = characterItems[ i ].increaseNumberBy(temp);

                            if (itemsToCreate != temp) {
                                characterItems[ i ].setWear(cos.AgeingSpeed);
                                characterItems[ i ].setQuality(quality);
                            }
                        }
                    }

                    if (temp > 0) {
                        it.setId(id);
                        it.setQuality(quality);
                        it.setWear(cos.AgeingSpeed);
                        it.setData(data);

                        if (backPackContents != NULL) {
                            bool ok = true;

                            while (ok && (temp > 0)) {
                                if (temp >= cos.MaxStack) {
                                    it.setNumber(cos.MaxStack);
                                } else {
                                    it.setNumber(temp);
                                }

                                auto leftOver = backPackContents->mergeItem(it);

                                if (leftOver > 0) {
                                    ok = false;
                                }

                                temp -= it.getNumber() - leftOver;
                            }
                        }
                    }
                }

                for (unsigned char i = MAX_BODY_ITEMS; i < MAX_BELT_SLOTS + MAX_BODY_ITEMS && temp > 0; ++i) {
                    if (characterItems[ i ].getId() == 0) {
                        characterItems[ i ].setId(id);
                        characterItems[ i ].setWear(cos.AgeingSpeed);
                        characterItems[ i ].setQuality(quality);
                        characterItems[ i ].setData(data);
                        temp = characterItems[ i ].increaseNumberBy(temp);
                    }
                }

                if (temp != old_temp && cos.Brightness > 0) {
                    updateAppearanceForAll(true);
                }

                if ((temp > 0) && (backPackContents != NULL)) {
#ifdef Character_DEBUG
                    std::cout << "createItem: Platz im belt nicht ausreichend, erstelle im backpack" << std::endl;
#endif
                    bool ok = true;
                    it.setId(id);
                    it.setQuality(quality);
                    it.setWear(cos.AgeingSpeed);
                    it.setData(data);

                    while (ok && (temp > 0)) {
                        if (temp >= cos.MaxStack) {
                            it.setNumber(cos.MaxStack);
                        } else {
                            it.setNumber(temp);
                        }

                        if (!backPackContents->InsertItem(it, true)) {
                            ok = false;
                        } else {
                            temp -= it.getNumber();
                        }
                    }
                }
            }
        }

#ifdef Character_DEBUG
        std::cout << "createItem: Anzahl der Item die nicht erstellt werden konnten: " << temp << std::endl;
#endif

    }

    return temp;

}


int Character::increaseAtPos(unsigned char pos, int count) {
    int temp = count;

#ifdef Character_DEBUG
    std::cout << "increaseAtPos " << (short int) pos << " " << count << "\n";
#endif

    if ((pos > 0) && (pos < MAX_BELT_SLOTS + MAX_BODY_ITEMS)) {
        if (weightOK(characterItems[ pos ].getId(), count, NULL)) {

            temp = characterItems[ pos ].getNumber() + count;

#ifdef Character_DEBUG
            std::cout << "temp " << temp << "\n";
#endif
            auto maxStack = characterItems[pos].getMaxStack();

            if (temp > maxStack) {
                characterItems[ pos ].setNumber(maxStack);
                temp = temp - maxStack;
            } else if (temp <= 0) {
                bool updateBrightness = World::get()->getItemStatsFromId(characterItems[ pos ].getId()).Brightness > 0;
                temp = count + characterItems[ pos ].getNumber();
                characterItems[ pos ].reset();

                if (updateBrightness) {
                    updateAppearanceForAll(true);
                }
            } else {
                characterItems[ pos ].setNumber(temp);
                temp = 0;
            }
        }
    }

    return temp;
}


bool Character::swapAtPos(unsigned char pos, TYPE_OF_ITEM_ID newid, uint16_t newQuality) {
#ifdef Character_DEBUG
    std::cout << "swapAtPos " << (short int) pos << " newid " << newid << "\n";
#endif

    if ((pos > 0) && (pos < MAX_BELT_SLOTS + MAX_BODY_ITEMS)) {
#ifdef Character_DEBUG
        std::cout << "pos gefunden, alte id: " << characterItems[ pos ].getId() << "\n";
#endif
        bool updateBrightness = World::get()->getItemStatsFromId(characterItems[ pos ].getId()).Brightness > 0 || World::get()->getItemStatsFromId(newid).Brightness > 0;
        characterItems[ pos ].setId(newid);

        if (updateBrightness) {
            updateAppearanceForAll(true);
        }

        if (newQuality > 0) {
            characterItems[ pos ].setQuality(newQuality);
        }

        return true;
    } else {
        return false;
    }
}


void Character::ageInventory() {
    CommonStruct tempCommon;

    for (unsigned char i = 0; i < MAX_BELT_SLOTS + MAX_BODY_ITEMS; ++i) {
        if (characterItems[ i ].getId() != 0) {
            if (!CommonItems->find(characterItems[ i ].getId(), tempCommon)) {
                tempCommon.rotsInInventory=false;
                tempCommon.ObjectAfterRot = characterItems[ i ].getId();
            }

            if (tempCommon.rotsInInventory) {
                if (!characterItems[ i ].survivesAgeing()) {
                    if (characterItems[ i ].getId() != tempCommon.ObjectAfterRot) {
#ifdef Character_DEBUG
                        std::cout << "INV:Ein Item wird umgewandelt von: " << characterItems[ i ].getId() << "  nach: " << tempCommon.ObjectAfterRot << "!\n";
#endif
                        characterItems[ i ].setId(tempCommon.ObjectAfterRot);

                        if (CommonItems->find(tempCommon.ObjectAfterRot, tempCommon)) {
                            characterItems[ i ].setWear(tempCommon.AgeingSpeed);
                        }
                    } else {
#ifdef Character_DEBUG
                        std::cout << "INV:Ein Item wird gel�cht,ID:" << characterItems[ i ].getId() << "!\n";
#endif
                        characterItems[ i ].reset();
                    }
                }
            }
        }
    }

    // Inhalt des Rucksacks altern
    if ((characterItems[ BACKPACK ].getId() != 0) && (backPackContents != NULL)) {
        backPackContents->doAge(true);
    }

    std::map<uint32_t, Container *>::iterator depotIterator;

    for (depotIterator = depotContents.begin(); depotIterator != depotContents.end(); ++depotIterator) {
        if (depotIterator->second != NULL) {
            depotIterator->second->doAge(true);
        }
    }
}

void Character::SetAlive(bool t) {
    bool wasAlive = alive;
    alive = t;

    if (wasAlive && !alive && (character == player)) {
        updateAppearanceForAll(true);

        Player *player = dynamic_cast<Player *>(this);
        player->ltAction->abortAction();

        if (playerDeathScript) {
            playerDeathScript->playerDeath(player);
        }
    }
}


bool Character::attack(Character *target) {

    if (target != NULL && target->IsAlive()) {

        if (!actionRunning()) {
            if (target->IsAlive()) {
                if (target->character == player) {
                    Player *pl = dynamic_cast<Player *>(target);
                    pl->ltAction->actionDisturbed(this);
                }

                callAttackScript(this, target);
            }
        }

        if (character == player) {
            if (target->IsAlive()) {
                boost::shared_ptr<BasicServerCommand>cmd(new BBSendActionTC(id, name, 1 , "Attacks : " + target->name + "(" + Logger::toString(target->id) + ")"));
                _world->monitoringClientList->sendCommand(cmd);
            } else {
                boost::shared_ptr<BasicServerCommand>cmd(new BBSendActionTC(id, name, 1 , "Killed : " + target->name + "(" + Logger::toString(target->id) + ")"));
                _world->monitoringClientList->sendCommand(cmd);
            }
        }

        if (!target->IsAlive()) {
            // target was killed...
            if (character == player || target->character == player) {
                // player killed something or was killed...
                time_t acttime = time(NULL);
                std::string killtime = ctime(&acttime);
                killtime[killtime.size()-1] = ':';
                kill_log << killtime << " ";

                switch (character) {
                case player:
                    kill_log << "Player " << name << "(" << id << ") ";
                    break;
                case monster:
                    kill_log << "Monster of race  " << race << "(" << id << ") ";
                    break;
                case npc:
                    kill_log << "NPC " << name << "(" << id << ") ";
                    break;
                }

                kill_log << "killed ";

                switch (target->character) {
                case player:
                    kill_log << "Player " << target->name << "(" << target->id << ") ";
                    break;
                case monster:
                    kill_log << "Monster of race  " << target->race << "(" << target->id << ") ";
                    break;
                case npc:
                    kill_log << "NPC " << target->name << "(" << target->id << ") ";
                    break;
                }

                kill_log << std::endl;

            }
        }

        return (target->IsAlive());
    }

    return false;
}

std::string Character::getSkillName(TYPE_OF_SKILL_ID s) {
    SkillStruct skillStruct;

    if (Skills->find(s, skillStruct)) {
        return skillStruct.englishName;
    } else {
        return "unknown skill";
    }
}

unsigned short int Character::getSkill(TYPE_OF_SKILL_ID s) {
    SKILLMAP::iterator iterator;
    iterator = skills.find(s);

    if (iterator == skills.end()) {
#ifdef Character_DEBUG
        std::cout << "getSkill: Skill " << s << " not found!\n";
#endif
        return 0;
    } else {
#ifdef Character_DEBUG
        std::cout << "getSkill: Skill " << s << " found! " << (*iterator).second.value << "\n";
#endif
        return (*iterator).second.major;
    }
}

unsigned short int Character::getMinorSkill(TYPE_OF_SKILL_ID s) {
    SKILLMAP::iterator iterator;
    iterator = skills.find(s);

    if (iterator == skills.end()) {
#ifdef Character_DEBUG
        std::cout << "getMinorSkill: Skill " << s << " not found!\n";
#endif
        return 0;
    } else {
#ifdef Character_DEBUG
        std::cout << "getMinorSkill: Skill " << s << " found! " << (*iterator).second.value << "\n";
#endif
        return (*iterator).second.minor;
    }
}


void Character::setSkinColor(uint8_t red, uint8_t green, uint8_t blue) {
    skinred=red;
    skingreen=green;
    skinblue=blue;
    updateAppearanceForAll(true);
}


void Character::getSkinColor(uint8_t &red, uint8_t &green, uint8_t &blue) {
    red=skinred;
    green=skingreen;
    blue=skinblue;
}


void Character::setHairColor(uint8_t red, uint8_t green, uint8_t blue) {
    hairred=red;
    hairgreen=green;
    hairblue=blue;
    updateAppearanceForAll(true);
}


void Character::getHairColor(uint8_t &red, uint8_t &green, uint8_t &blue) {
    red=hairred;
    green=hairgreen;
    blue=hairblue;
}


void Character::setHair(uint8_t hairID) {
    hair=hairID;
    updateAppearanceForAll(true);
}


uint8_t Character::getHair() {
    return hair;
}


void Character::setBeard(uint8_t beardID) {
    beard=beardID;
    updateAppearanceForAll(true);
}


uint8_t Character::getBeard() {
    return beard;
}

void Character::setAttribute(Character::attributeIndex attribute, Attribute::attribute_t value) {
    auto &attrib = *(attributes[attribute]);
    auto oldValue = attrib.getValue();
    attrib.setValue(value);
    auto newValue = attrib.getValue();

    if (newValue != oldValue) {
        handleAttributeChange(attribute);
    }
}

Attribute::attribute_t Character::getAttribute(Character::attributeIndex attribute) const {
    return attributes[attribute]->getValue();
}

Attribute::attribute_t Character::increaseAttribute(Character::attributeIndex attribute, int amount) {
    auto &attrib = *(attributes[attribute]);
    auto oldValue = attrib.getValue();
    attrib.increaseValue(amount);
    auto newValue = attrib.getValue();

    if (newValue != oldValue) {
        handleAttributeChange(attribute);
    }

    return newValue;
}

void Character::handleAttributeChange(Character::attributeIndex attribute) {
    if (attribute == Character::hitpoints) {
        SetAlive(getAttribute(hitpoints) > 0);
        _world->sendHealthToAllVisiblePlayers(this, getAttribute(hitpoints));
    }
}

void Character::setAttrib(std::string name, Attribute::attribute_t value) {
    if (name == "faceto") {
        turn((direction)value);
    } else if (name == "racetyp") {
        race = (race_type)value;
        updateAppearanceForAll(true);
    } else {
        try {
            Character::attributeIndex attribute = attributeMap.at(name);
            setAttribute(attribute, value);
        } catch (...) {

        }
    }
}

Attribute::attribute_t Character::increaseAttrib(std::string name, int amount) {
    if (name == "posx") {
        return pos.x;
    }

    if (name == "posy") {
        return pos.y;
    }

    if (name == "posz") {
        return pos.z;
    }

    if (name == "faceto") {
        return faceto;
    }

    if (name == "id") {
        return id;
    }

    if (name == "racetyp") {
        return race;
    }

    if (name == "sex") {
        return getAttribute(Character::sex);
    }

    if (name == "magictype") {
        return magic.type;
    }

    try {
        Character::attributeIndex attribute = attributeMap.at(name);
        return increaseAttribute(attribute, amount);
    } catch (...) {

    }

    return 0;
}

unsigned short int Character::setSkill(TYPE_OF_SKILL_ID skill, short int major, short int minor) {
    if (!Skills->find(skill)) {
        return 0;
    }

    SKILLMAP::iterator iterator;
    iterator = skills.find(skill);
    {
        if (iterator == skills.end()) {
            skillvalue sv;
            sv.major = major;
            sv.minor = minor;
            skills[skill] = sv;
            return sv.major;
        } else {
            iterator->second.major = major;
            iterator->second.minor = minor;
            return iterator->second.major;
        }
    }
}

unsigned short int Character::increaseSkill(TYPE_OF_SKILL_ID skill, short int amount) {
    if (!Skills->find(skill)) {
        return 0;
    }

    SKILLMAP::iterator iterator;
    iterator = skills.find(skill);

    if (iterator == skills.end()) {
        skillvalue sv;

        if (amount <= 0) {
            return 0; //Don't add new skill if value <= 0
        } else if (amount > MAJOR_SKILL_GAP) {
            sv.major = MAJOR_SKILL_GAP;
        } else {
            sv.major = amount;
        }

        skills[skill] = sv;
        return sv.major;
    } else {
        int temp=iterator->second.major + amount;

        if (temp <= 0) {
            iterator->second.major = 0;
            skills.erase(iterator); //L�chen des Eintrags wenn value <= 0
        } else if (temp > MAJOR_SKILL_GAP) {
            iterator->second.major = MAJOR_SKILL_GAP;
        } else {
            iterator->second.major = temp;
        }

        return (iterator->second.major);
    }
}


unsigned short int Character::increaseMinorSkill(TYPE_OF_SKILL_ID skill, short int amount) {
    if (!Skills->find(skill)) {
        return 0;
    }

    SKILLMAP::iterator iterator;
    iterator = skills.find(skill);

    if (iterator == skills.end()) {
        skillvalue sv;

        if (amount <= 0) {
            return 0; //Don't add new skill if value <= 0
        } else if (amount > 10000) {
            sv.minor = 10000;
        } else {
            sv.minor = amount;
        }

        if (sv.minor >= 10000) {
            sv.minor = 0;
            sv.major++;
        }

        skills[ skill ] = sv;
        return (sv.major);
    } else {
        int temp=iterator->second.minor + amount;

        if (temp <= 0) {
            iterator->second.minor = 0;
            iterator->second.major--;

            if (iterator->second.major==0) {
                skills.erase(iterator);    //delete if major == 0
            }
        } else if (temp >= 10000) {
            iterator->second.minor = 0;
            iterator->second.major++;

            if (iterator->second.major > MAJOR_SKILL_GAP) {
                iterator->second.major = MAJOR_SKILL_GAP;
            }
        } else {
            iterator->second.minor = temp;
        }

        return (iterator->second.major);
    }
}

Character::skillvalue *Character::getSkillValue(TYPE_OF_SKILL_ID s) {
    SKILLMAP::iterator it = skills.find(s);

    if (it == skills.end()) {
        return NULL;
    } else {
        return &(it->second);
    }
}

void Character::learn(TYPE_OF_SKILL_ID skill, uint32_t actionPoints, uint8_t opponent) {
    if (!Skills->find(skill)) {
        return;
    }

    if (learnScript) {
        learnScript->learn(this, skill, actionPoints, opponent);
    } else {
        std::cerr<<"learn called but script was not initialized"<<std::endl;
    }
}


void Character::deleteAllSkills() {
    skills.clear();
}


bool Character::isInRange(Character *cc, unsigned short int distancemetric) {
    if (cc != NULL) {
        short int pz = cc->pos.z - pos.z;
        short int px = cc->pos.x - pos.x;
        short int py = cc->pos.y - pos.y;

        if (((Abso(px) + Abso(py)) <= distancemetric) && (pz==0)) {
            return true;
        }
    }

    return false;
}

unsigned short int Character::getScreenRange() const {
    return 14;
}

bool Character::isInRangeToField(position m_pos, unsigned short int distancemetric) {
    short int pz = m_pos.z - pos.z;
    short int px = m_pos.x - pos.x;
    short int py = m_pos.y - pos.y;

    if (((Abso(px) + Abso(py)) <= distancemetric) && (pz == 0)) {
        return true;
    } else {
        return false;
    }
}

unsigned short int Character::distanceMetricToPosition(position m_pos) {
    unsigned short int ret=0xFFFF;
    short int pz = pos.z - m_pos.z;
    short int px = pos.x - m_pos.x;
    short int py = pos.y - m_pos.y;

    if (pz > 0) {
        ret = pz;
    } else {
        ret = 0 - pz;
    }

    if (px > 0) {
        if (px > ret) {
            ret = px;
        }
    } else {
        if ((0 - px) > ret) {
            ret = 0 - px;
        }
    }

    if (py > 0) {
        if (py > ret) {
            ret = py;
        }
    } else {
        if ((0 - py) > ret) {
            ret = 0 - py;
        }
    }

    return ret;
}

unsigned short int Character::distanceMetric(Character *cc) {
    unsigned short int ret=0xFFFF;

    if (cc != NULL) {
        short int pz = pos.z - cc->pos.z;
        short int px = pos.x - cc->pos.x;
        short int py = pos.y - cc->pos.y;

        if (pz > 0) {
            ret = pz;
        } else {
            ret = 0 - pz;
        }

        if (px > 0) {
            if (px > ret) {
                ret = px;
            }
        } else {
            if ((0 - px) > ret) {
                ret = 0 - px;
            }
        }

        if (py > 0) {
            if (py > ret) {
                ret = py;
            }
        } else {
            if ((0 - py) > ret) {
                ret = 0 - py;
            }
        }
    }

    return ret;
}


unsigned short int Character::maxLiftWeigt() {
    return 29999;
}


unsigned short int Character::maxLoadWeight() {
    return getAttribute(Character::strength) * 500 + 5000;
}


int Character::LoadWeight() {
    int load=0;

    // alle Items bis auf den Rucksack
    for (int i=1; i < MAX_BODY_ITEMS + MAX_BELT_SLOTS; ++i) {
        load += weightItem(characterItems[i].getId(),characterItems[i].getNumber());
    }

    // Rucksack
    load += weightContainer(characterItems[0].getId(), 1, backPackContents);

    if (load > 30000) {
        return 30000;
    } else if (load < 0) {
        return 0;
    } else {
        return load;
    }
}


bool Character::weightOK(TYPE_OF_ITEM_ID id, int count, Container *tcont) {
    bool ok;

    int realweight = LoadWeight();

    if (tcont != NULL) {
        ok = (realweight + weightContainer(id, 1, tcont)) <= maxLoadWeight();
    } else {
        ok = (realweight + weightItem(id, count)) <= maxLoadWeight();
    }

    return ok;

}


int Character::Abso(int value) {
    if (value < 0) {
        return (0 - value);
    }

    return value;
}


int Character::weightItem(TYPE_OF_ITEM_ID id, int count) {
    int gweight;

    if (CommonItems->find(id, tempCommon)) {
        gweight = tempCommon.Weight * count;
    } else {
        gweight = 0;
    }

    if (gweight > 30000) {
        return 30000;
    } else {
        return gweight;
    }
}


int Character::weightContainer(TYPE_OF_ITEM_ID id, int count, Container *tcont) {
    int temp=0;

    if (id != 0) {
        if (CommonItems->find(id, tempCommon)) {
            if (count > 0) {
                temp = tempCommon.Weight;
            } else {
                temp = 0 - tempCommon.Weight;
            }
        }

        if (tcont != NULL) {
            try {
                if (count > 0) {
                    temp += tcont->weight();
                } else {
                    temp -= tcont->weight();
                }
            } catch (RekursionException &e) {
                std::cerr << "weightContainer: maximale Rekursionstiefe " << MAXIMALEREKURSIONSTIEFE << " wurde bei Char " << name << " ueberschritten!" << std::endl;
                return 30000;
            }
        }
    }

    if (temp > 30000) {
        return 30000;
    } else {
        return temp;
    }
}

Character::movement_type Character::GetMovement() {
    return _movement;
}


void Character::SetMovement(movement_type tmovement) {
    _movement = tmovement;
}

void Character::increasePoisonValue(short int value) {
    if ((poisonvalue + value) >= MAXPOISONVALUE) {
        poisonvalue = MAXPOISONVALUE;
    } else if ((poisonvalue + value) <= 0) {
        poisonvalue = 0;
    } else {
        poisonvalue += value;
    }
}

void Character::increaseMentalCapacity(int value) {
    if ((mental_capacity + value) <= 0) {
        mental_capacity = 0;
    } else {
        mental_capacity += value;
    }
}

std::string Character::alterSpokenMessage(std::string message, int languageSkill) {
    int counter=0;
    std::string alteredMessage;

    alteredMessage=message;

    while (message[counter]!=0) {
        if (Random::uniform(0,70)>languageSkill) {
            alteredMessage[counter]='*';
        }

        counter++;
    }

    //std::cout << "message: "<< message << ", altered msg: " << alteredMessage << "\n";

    return alteredMessage;
}

int Character::getLanguageSkill(int languageSkillNumber) {
    return 100;
}

void Character::talk(talk_type tt, std::string message) { //only for say, whisper, shout
    std::string talktype;
    uint16_t cost = 0;
    lastSpokenText=message;

    switch (tt) {
    case tt_say:

        if (!IsAlive()) {
            return;
        }

#ifdef DO_UNCONSCIOUS

        if (!IsConscious()) {
            return;
        }

#endif
        talktype = "says";
        cost = P_SAY_COST;
        break;
    case tt_whisper:
#ifdef DO_UNCONSCIOUS

        if (!IsConscious()) {
            return;
        }

#endif
        talktype = "whispers";
        cost = P_WHISPER_COST;
        break;
    case tt_yell:

        if (!IsAlive()) {
            return;
        }

#ifdef DO_UNCONSCIOUS

        if (!IsConscious()) {
            return;
        }

#endif
        talktype = "shouts";
        cost = P_SHOUT_COST;
        break;
    }

#ifdef LOG_TALK

    // log talk if we have a player
    if (character == player) {
        time_t acttime = time(NULL);
        std::string talktime = ctime(&acttime);
        talktime[talktime.size()-1] = ':';
        talkfile << talktime << " ";
        talkfile << name << "(" << id << ") " << talktype << ": " << message << std::endl;
    }

#endif

    if (character == player) {
        /**
         * create a new Talk command and send them
         */
        boost::shared_ptr<BasicServerCommand>cmd(new BBTalkTC(id ,name, static_cast<unsigned char>(tt), message));
        _world->monitoringClientList->sendCommand(cmd);
    }



    _world->sendMessageToAllCharsInRange(message,tt,this);
    actionPoints -= cost;
}

void Character::talkLanguage(talk_type tt, unsigned char lang, std::string message) {
    uint16_t cost = 0;
    lastSpokenText=message;

    switch (tt) {
    case tt_say:

        if (!IsAlive()) {
            return;
        }

#ifdef DO_UNCONSCIOUS

        if (!IsConscious()) {
            return;
        }

#endif
        cost = P_SAY_COST;
        break;
    case tt_whisper:
#ifdef DO_UNCONSCIOUS

        if (!IsConscious()) {
            return;
        }

#endif
        cost = P_WHISPER_COST;
        break;
    case tt_yell:

        if (!IsAlive()) {
            return;
        }

#ifdef DO_UNCONSCIOUS

        if (!IsConscious()) {
            return;
        }

#endif
        cost = P_SHOUT_COST;
        break;
    }

    _world->sendLanguageMessageToAllCharsInRange(message,tt,lang,this);
    actionPoints -= cost;
}

void Character::turn(direction dir) {
    if (dir != dir_up && dir != dir_down && dir != static_cast<Character::direction>(faceto)) {
        faceto = (Character::face_to)dir;
        _world->sendSpinToAllVisiblePlayers(this);
    }
}

void Character::turn(position posi) {
    //attack the player which we have found
    short int xoffs = posi.x - pos.x;
    short int yoffs = posi.y - pos.y;

    if (abs(xoffs)>abs(yoffs)) {
        turn(static_cast<Character::direction>((xoffs>0)?2:6));
    } else {
        turn(static_cast<Character::direction>((yoffs>0)?4:0));
    }
}

bool Character::move(direction dir, bool active) {
    //Ggf Scriptausfhrung wenn man sich von einen Feld wegbewegt.
    _world->TriggerFieldMove(this,false);

    // if we move we look into that direction...
    if (dir != dir_up && dir != dir_down) {
        faceto = (Character::face_to)dir;
    }

    // check if we can move to our target field
    position newpos = pos;
    newpos.x += _world->moveSteps[ dir ][ 0 ];
    newpos.y += _world->moveSteps[ dir ][ 1 ];
    newpos.z += _world->moveSteps[ dir ][ 2 ];

    bool fieldfound = false;
    Field *cfnew, *cfold;

    // get the old tile... we need it to update the old tile as well as for the walking cost
    _world->GetPToCFieldAt(cfold, pos.x, pos.y, pos.z);

    // we need to search for tiles below this level
    for (size_t i = 0; i < RANGEDOWN + 1 && !fieldfound; ++i) {
        fieldfound = _world->GetPToCFieldAt(cfnew, newpos.x, newpos.y, newpos.z);

        // did we hit a targetfield?
        if (!fieldfound || cfnew->getTileId() == TRANSPARENTDISAPPEAR || cfnew->getTileId() == TRANSPARENT) {
            fieldfound = false;
            --newpos.z;
        }
    }

    // did we find a target field?
    if (fieldfound && moveToPossible(cfnew)) {
        uint16_t movementcost = getMovementCost(cfnew);
        int16_t diff = (P_MIN_AP - actionPoints + movementcost) * 10;
        uint8_t waitpages;

        // necessay to get smooth movement in client (dunno how this one is supposed to work exactly)
        if (diff < 60) {
            waitpages = 4;
        } else {
            waitpages = (diff * 667) / 10000;
        }

        actionPoints -= movementcost;

        // mark fields as (un)occupied
        cfold->removeChar();
        cfnew->setChar();

        // set new position
        updatePos(newpos);

        // send word out to all chars in range
        if (active) {
            _world->sendCharacterMoveToAllVisibleChars(this, waitpages);
        } else {
            _world->sendPassiveMoveToAllVisiblePlayers(this);
        }

        // check if there are teleporters or other special flags on this field
        _world->checkFieldAfterMove(this, cfnew);

        // ggf Scriptausfhrung nachdem man sich auf das Feld drauf bewegt hat
        _world->TriggerFieldMove(this,true);

        return true;
    }

    return false;
}

bool Character::moveToPossible(const Field *field) {
    // for monsters/npcs we just use the field infos for now
    return field->moveToPossible();
}

uint16_t Character::getMovementCost(Field *sourcefield) {
    uint16_t walkcost = 0;
    auto tileId = sourcefield->getTileId();

    if (!Tiles->find(tileId, tempTile)) {
        std::cerr<<"no move cost for tile: " << tileId << std::endl;
        return walkcost;
    }

    if (tempTile.flags & FLAG_PASSABLE) {
        tileId = sourcefield->getSecondaryTileId();
    }

    if (!Tiles->find(tileId, tempTile)) {
        std::cerr<<"no move cost for tile: " << tileId << std::endl;
        return walkcost;
    }

    switch (_movement) {
    case walk:
        walkcost += tempTile.walkingCost;
        break;
    case fly: // walking cost independent of source field
        walkcost += NP_STANDARDFLYCOST;
        break;
    case crawl: // just double the ap necessary for walking
        walkcost += 2 * tempTile.walkingCost;
        break;
    }

    if (character != player) {
        walkcost += STANDARD_MONSTER_WALKING_COST;
    }

    walkcost = (walkcost * P_MOVECOSTFORMULA_walkingCost_MULTIPLIER) / (getAttribute(Character::agility) + P_MOVECOSTFORMULA_agility_ADD);

    return walkcost;
}

void Character::updatePos(position newpos) {
    pos = newpos;
}

void Character::receiveText(talk_type tt, std::string message, Character *cc) {
    // overloaded where necessary
}

void Character::introducePlayer(Player *) {
    // overloaded in Player
}

void Character::teachMagic(unsigned char type, unsigned char flag) {
    // overloaded in Player
}

bool Character::Warp(position newPos) {
    position oldpos = pos;
    Field *fold = NULL;

    if (_world->GetPToCFieldAt(fold, pos.x, pos.y, pos.z)) {
        Field *fnew = NULL;

        if (_world->findEmptyCFieldNear(fnew, newPos.x, newPos.y, newPos.z)) {
            fold->removeChar();
            updatePos(newPos);
            fnew->setChar();
            _world->sendCharacterWarpToAllVisiblePlayers(this, oldpos, PUSH);
            return true;
        } else {
            std::cout<< "Characterwarp, Zielfeld nicht gefunden! "<<std::endl;
            return false;
        }

    } else {
        std::cout<< "Characterwarp, Quellfeld nicht gefunden! "<<std::endl;
        return false;
    }

    return false;
}

bool Character::forceWarp(position newPos) {
    position oldpos = pos;
    Field *fold = NULL;

    if (_world->GetPToCFieldAt(fold, pos.x, pos.y, pos.z)) {
        Field *fnew = NULL;

        if (_world->GetPToCFieldAt(fnew, newPos.x, newPos.y, newPos.z)) {
            fold->removeChar();
            updatePos(newPos);
            fnew->setChar();
            _world->sendCharacterWarpToAllVisiblePlayers(this, oldpos, PUSH);
            return true;
        } else {
            std::cout<< "forceWarp, Zielfeld nicht gefunden! "<<std::endl;
            return false;
        }

    } else {
        std::cout<< "forceWarp, Quellfeld nicht gefunden! "<<std::endl;
        return false;
    }

    return false;
}

void Character::LTIncreaseHP(unsigned short int value, unsigned short int count, unsigned short int time) {
    //Nothing to do here, overloaded for players
}

void Character::LTIncreaseMana(unsigned short int value, unsigned short int count, unsigned short int time) {
    //Nothing to do here, overloaded for players
}

void Character::Depot() {
    //Nothing to do here, overloaded for players
}

void Character::startMusic(short int title) {
    //Nothing to do here, overloaded for players
}

void Character::defaultMusic() {
    //Nothing to do here, overloaded for players
}

void Character::inform(std::string text, informType type) {
    // override for char types that need this kind of information
}

void Character::informLua(std::string text) {
    // override for char types that need this kind of information
}

void Character::informLua(std::string german, std::string english) {
    // override for char types that need this kind of information
}

void Character::informLua(std::string text, informType type) {
    // override for char types that need this kind of information
}

void Character::informLua(std::string german, std::string english, informType type) {
    // override for char types that need this kind of information
}

void Character::changeQualityItem(TYPE_OF_ITEM_ID id, short int amount) {
    if ((characterItems[ BACKPACK ].getId() != 0) && (backPackContents != NULL)) {
        if (backPackContents->changeQuality(id, amount)) {
            return;
        }

        //�dern des Items in eine untercontainer geschehen.
    }

    short int tmpQuality;

    for (unsigned char i = MAX_BELT_SLOTS + MAX_BODY_ITEMS - 1; i > 0; --i) {
        if (characterItems[ i ].getId() == id) {
            // don't increase the class of the item, but allow decrease of the item class
            tmpQuality = ((amount+characterItems[i].getDurability())<100) ? (amount + characterItems[i].getQuality()) : (characterItems[i].getQuality() - characterItems[i].getDurability() + 99);

            if (tmpQuality%100 > 1) {
                characterItems[i].setQuality(tmpQuality);
                return;
            } else {
                if (i == RIGHT_TOOL && characterItems[LEFT_TOOL].getId() == BLOCKEDITEM) {
                    //Belegt aus linker hand l�chen wenn item in rechter hand ein zweih�deritem war
                    characterItems[LEFT_TOOL].reset();
                } else if (i == LEFT_TOOL && characterItems[RIGHT_TOOL].getId() == BLOCKEDITEM) {
                    //Belegt aus rechter hand l�chen wenn item in linker hand ein zweih�der ist
                    characterItems[RIGHT_TOOL].reset();
                }

                characterItems[i].reset();
                return;
            }
        }
    }
}

void Character::changeQualityAt(unsigned char pos, short int amount) {
    std::cout<<"In ChangeQualityAt, pos: "<<(int)pos<<" amount: "<<amount<<" !"<<std::endl;

    if (pos < MAX_BODY_ITEMS + MAX_BELT_SLOTS) {
        //Prfen ob berhaupt ein Item an der Stelle ist oder ein belegt
        if ((characterItems[ pos ].getId() == 0) || (characterItems[pos].getId() == BLOCKEDITEM)) {
            std::cerr<<"changeQualityAt, kein Item oder belegt an der position: "<<(int)pos<<" !"<<std::endl;
            return;
        }


        short int tmpQuality = ((amount+characterItems[pos].getDurability())<100) ? (amount + characterItems[pos].getQuality()) : (characterItems[pos].getQuality() - characterItems[pos].getDurability() + 99);

        if (tmpQuality%100 > 1) {
            std::cout<<"Qualit� des Items > 0"<<std::endl;
            characterItems[ pos ].setQuality(tmpQuality);
            std::cout<<"Akt Qualit�: "<<characterItems[ pos ].getQuality()<<std::endl;
            return;
        }
        //L�chen falls qualit� zu gering
        else {

            if (pos == RIGHT_TOOL && characterItems[LEFT_TOOL].getId() == BLOCKEDITEM) {
                //Belegt aus linker hand l�chen wenn item in rechter hand ein zweih�deritem war
                characterItems[LEFT_TOOL].reset();
            } else if (pos == LEFT_TOOL && characterItems[RIGHT_TOOL].getId() == BLOCKEDITEM) {
                //Belegt aus rechter hand l�chen wenn item in linker hand ein zweih�der ist
                characterItems[RIGHT_TOOL].reset();
            }

            characterItems[ pos ].reset();
            return;
        }
    }
}

void Character::callAttackScript(Character *Attacker, Character *Defender) {
    if (characterItems[ RIGHT_TOOL ].getId() != 0) {
        WeaponStruct tmpWeapon;

        if (WeaponItems->find(characterItems[ RIGHT_TOOL ].getId() , tmpWeapon)) {
            if (tmpWeapon.script && tmpWeapon.script->existsEntrypoint("onAttack")) {
                tmpWeapon.script->onAttack(Attacker, Defender);
            }
        }
    }

    standardFightingScript->onAttack(Attacker, Defender);
}

void Character::setQuestProgress(TYPE_OF_QUEST_ID questid, TYPE_OF_QUESTSTATUS progress) {
    // Nothing to do here, overridden for players
}

TYPE_OF_QUESTSTATUS Character::getQuestProgress(TYPE_OF_QUEST_ID questid) throw() {
    // Nothing to do here, overridden for players
    return 0;
}

luabind::object Character::getItemList(TYPE_OF_ITEM_ID id) {
    lua_State *_luaState = _world->getCurrentScript()->getLuaState();
    luabind::object list = luabind::newtable(_luaState);
    int index = 1;

    for (unsigned char i = 0; i < MAX_BELT_SLOTS + MAX_BODY_ITEMS; ++i) {
        if (characterItems[ i ].getId() == id) {
            ScriptItem item = characterItems[ i ];

            if (i < MAX_BODY_ITEMS) {
                item.type = ScriptItem::it_inventory;
            } else {
                item.type = ScriptItem::it_belt;
            }

            item.pos = pos;
            item.itempos = i;
            item.owner = this;
            list[index] = item; //ad an item to the index
            index++; //increase index after adding an item.
        }
    }

    // Inhalt des Rucksacks altern
    if ((characterItems[ BACKPACK ].getId() != 0) && (backPackContents != NULL)) {
        backPackContents->increaseItemList(id, list, index);
    }

    return list;
}


Container *Character::GetBackPack() {
    return backPackContents;
}

Container *Character::GetDepot(uint32_t depotid) {
    std::map<uint32_t, Container *>::iterator it;

    if ((it=depotContents.find(depotid + 1)) == depotContents.end()) {
        return 0;
    } else {
        return it->second;
    }
}


uint32_t Character::idleTime() {
    // Nothing to do here, overloaded in Player
    return 0;
}


void Character::sendBook(uint16_t bookID) {
    // Nothing to do here, overloaded in Player
}

void Character::requestInputDialog(InputDialog *inputDialog) {
    // Nothing to do here, overloaded in Player
}

void Character::requestMessageDialog(MessageDialog *messageDialog) {
    // Nothing to do here, overloaded in Player
}

void Character::requestMerchantDialog(MerchantDialog *merchantDialog) {
    // Nothing to do here, overloaded in Player
}

void Character::requestSelectionDialog(SelectionDialog *selectionDialog) {
    // Nothing to do here, overloaded in Player
}

void Character::requestCraftingDialog(CraftingDialog *craftingDialog) {
    // Nothing to do here, overloaded in Player
}

void Character::requestCraftingLookAt(unsigned int dialogId, ItemLookAt &lookAt) {
    // Nothing to do here, overloaded in Player
}

void Character::requestCraftingLookAtIngredient(unsigned int dialogId, ItemLookAt &lookAt) {
    // Nothing to do here, overloaded in Player
}

void Character::updateAppearanceForPlayer(Player *target, bool always) {
    if (!isinvisible) {
        boost::shared_ptr<BasicServerCommand> cmd(new AppearanceTC(this, target));
        target->sendCharAppearance(id, cmd, always);
    }
}

void Character::updateAppearanceForAll(bool always) {
    if (!isinvisible) {
        std::vector < Player * > temp = World::get()->Players.findAllCharactersInScreen(pos.x, pos.y, pos.z);
        std::vector < Player * > ::iterator titerator;

        for (titerator = temp.begin(); titerator < temp.end(); ++titerator) {
            boost::shared_ptr<BasicServerCommand> cmd(new AppearanceTC(this, *titerator));
            (*titerator)->sendCharAppearance(id, cmd, always);
        }
    }
}

void Character::forceUpdateAppearanceForAll() {
    updateAppearanceForAll(true);
}

void Character::sendCharDescription(TYPE_OF_CHARACTER_ID id,const std::string &desc) {
    //Nothing to do here, overloaded in Player
}

void Character::performAnimation(uint8_t animID) {
    if (!isinvisible) {
        boost::shared_ptr<BasicServerCommand> cmd(new AnimationTC(id, animID));

        std::vector < Player * > temp = World::get()->Players.findAllCharactersInScreen(pos.x, pos.y, pos.z);
        std::vector < Player * > ::iterator titerator;

        for (titerator = temp.begin(); titerator < temp.end(); ++titerator) {
            (*titerator)->Connection->addCommand(cmd);
        }
    }
}

bool Character::pageGM(std::string ticket) {
    //Nothing to do here, overloaded in Player

    return false;
}
