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
#include "data/Data.hpp"
#include "Container.hpp"
#include "World.hpp"
#include "data/TilesTable.hpp"
#include "script/LuaWeaponScript.hpp"
#include "Logger.hpp"
#include <map>
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include "script/LuaLearnScript.hpp"
#include "script/LuaPlayerDeathScript.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "fuse_ptr.hpp"
#include "a_star.hpp"

#define MAJOR_SKILL_GAP 100
#define USE_LUA_FIGTHING

extern std::shared_ptr<LuaLearnScript>learnScript;
extern std::shared_ptr<LuaPlayerDeathScript>playerDeathScript;

Character::attribute_map_t Character::attributeMap = {
    {"strength", strength},
    {"dexterity", dexterity},
    {"constitution", constitution},
    {"agility", agility},
    {"intelligence", intelligence},
    {"perception", perception},
    {"willpower", willpower},
    {"essence", essence},
    {"hitpoints", hitpoints},
    {"mana", mana},
    {"foodlevel", foodlevel},
    {"sex", sex},
    {"age", age},
    {"weight", weight},
    {"height", height},
    {"attitude", attitude},
    {"luck", luck}
};

Character::attribute_string_map_t Character::attributeStringMap = {
    {strength, "strength"},
    {dexterity, "dexterity"},
    {constitution, "constitution"},
    {agility, "agility"},
    {intelligence, "intelligence"},
    {perception, "perception"},
    {willpower, "willpower"},
    {essence, "essence"},
    {hitpoints, "hitpoints"},
    {mana, "mana"},
    {foodlevel, "foodlevel"},
    {sex, "sex"},
    {age, "age"},
    {weight, "weight"},
    {height, "height"},
    {attitude, "attitude"},
    {luck, "luck"}
};

position Character::getFrontalPosition() const {
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

bool Character::getStepList(const position &goal, std::list<direction> &steps) const {
    return pathfinding::a_star(pos, goal, steps);
}



bool Character::getNextStepDir(const position &goal, direction &dir) const {
    std::list<direction> steps;

    getStepList(goal, steps);

    if (!steps.empty()) {
        dir = steps.front();
        return true;
    } else {
        return false;
    }
}


Character::Character(const appearance &appearance) : actionPoints(P_MAX_AP), fightPoints(P_MAX_FP), effects(this), waypoints(this), _is_on_route(false), _world(World::get()), _appearance(appearance), attributes(ATTRIBUTECOUNT) {
    race = human;
    character = player;

    isinvisible=false;
    SetAlive(true);
    attackmode = false;
    poisonvalue = 0;
    mental_capacity = 0;
    _movement = walk;

    activeLanguage=0; //common language
    lastSpokenText="";

    pos.x = 0;
    pos.y = 0;
    pos.z = 0;

    for (int i = 0; i < MAX_BODY_ITEMS + MAX_BELT_SLOTS; ++i) {
        characterItems[ i ].reset();
    }

    attributes[strength] = Attribute(0, MAXATTRIB);
    attributes[dexterity] = Attribute(0, MAXATTRIB);
    attributes[constitution] = Attribute(0, MAXATTRIB);
    attributes[agility] = Attribute(0, MAXATTRIB);
    attributes[intelligence] = Attribute(0, MAXATTRIB);
    attributes[perception] = Attribute(0, MAXATTRIB);
    attributes[willpower] = Attribute(0, MAXATTRIB);
    attributes[essence] = Attribute(0, MAXATTRIB);
    attributes[hitpoints] = Attribute(0, MAXHPS);
    attributes[mana] = Attribute(0, MAXMANA);
    attributes[foodlevel] = Attribute(0, MAXFOOD);
    attributes[sex] = Attribute(0);
    attributes[age] = Attribute(0);
    attributes[weight] = Attribute(0);
    attributes[height] = Attribute(0);
    attributes[attitude] = Attribute(0);
    attributes[luck] = Attribute(0);

    faceto = north;
    backPackContents = nullptr;

    magic.type = MAGE;
    magic.flags[ MAGE ] = 0x00000000;
    magic.flags[ PRIEST ] = 0x00000000;
    magic.flags[ BARD ] = 0x00000000;
    magic.flags[ DRUID ] = 0x00000000;
}

Character::~Character() {
    //blow lua fuse for this char
    fuse_ptr<Character>::blow_fuse(this);

    if (backPackContents) {
        delete backPackContents;
        backPackContents = nullptr;
    }

    std::map<uint32_t,Container *>::reverse_iterator rit;

    for (rit = depotContents.rbegin(); rit != depotContents.rend(); ++rit) {
        delete rit->second;
    }
}


int Character::countItem(TYPE_OF_ITEM_ID itemid) const {
    int temp = 0;

    for (unsigned char i = 0; i < MAX_BELT_SLOTS + MAX_BODY_ITEMS; ++i) {
        if (characterItems[ i ].getId() == itemid) {
            temp = temp + characterItems[ i ].getNumber();
        }
    }

    if ((characterItems[ BACKPACK ].getId() != 0) && backPackContents) {
        temp = temp + backPackContents->countItem(itemid);
    }

    return temp;
}

int Character::countItemAt(const std::string &where, TYPE_OF_ITEM_ID itemid, script_data_exchangemap const *data) const {
    int temp = 0;

    if (where == "all") {
        for (unsigned char i = 0; i < MAX_BELT_SLOTS + MAX_BODY_ITEMS; ++i) {
            if (characterItems[ i ].getId() == itemid && (data == nullptr || characterItems[i].hasData(*data))) {
                temp = temp + characterItems[ i ].getNumber();
            }
        }

        if ((characterItems[ BACKPACK ].getId() != 0) && backPackContents) {
            temp = temp + backPackContents->countItem(itemid, data);
        }

        return temp;
    }

    if (where == "belt") {
        for (unsigned char i = MAX_BODY_ITEMS; i < MAX_BODY_ITEMS + MAX_BELT_SLOTS; ++i) {
            if (characterItems[ i ].getId() == itemid && (data == nullptr || characterItems[i].hasData(*data))) {
                temp = temp + characterItems[ i ].getNumber();
            }
        }

        return temp;
    }

    if (where == "body") {
        for (unsigned char i = 0; i < MAX_BODY_ITEMS; ++i) {
            if (characterItems[ i ].getId() == itemid && (data == nullptr || characterItems[i].hasData(*data))) {
                temp = temp + characterItems[ i ].getNumber();
            }
        }

        return temp;
    }

    if (where == "backpack") {
        if ((characterItems[ BACKPACK ].getId() != 0) && backPackContents) {
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


int Character::eraseItem(TYPE_OF_ITEM_ID itemid, int count, script_data_exchangemap const *data) {
    int temp = count;

    if ((characterItems[ BACKPACK ].getId() != 0) && backPackContents) {
        temp = backPackContents->eraseItem(itemid, temp, data);
    }

    if (temp > 0) {
        // BACKPACK als Item erstmal auslassen
        for (unsigned char i = MAX_BELT_SLOTS + MAX_BODY_ITEMS - 1; i > 0; --i) {
            if ((characterItems[ i ].getId() == itemid && (data == nullptr || characterItems[ i ].hasData(*data))) && (temp > 0)) {
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

    return temp;
}

int Character::createAtPos(unsigned char pos, TYPE_OF_ITEM_ID newid, int count) {
    int temp = count;
    Item it;

    if (weightOK(newid, count, nullptr)) {
        const auto &cos = Data::CommonItems[newid];

        if (cos.isValid()) {
            if (!Data::ContainerItems.exists(newid)) {
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


int Character::createItem(Item::id_type id, Item::number_type number, Item::quality_type quality, script_data_exchangemap const *data) {
    int temp = number;
    Item it;

    if (weightOK(id, number, nullptr)) {
        const auto &cos = Data::CommonItems[id];

        if (cos.isValid()) {
            if (Data::ContainerItems.exists(id)) {
                if (characterItems[ BACKPACK ].getId() == 0) {
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
                    if (!backPackContents->InsertContainer(it, new Container(it.getId()))) {
                        i = 0;
                    } else {
                        temp = temp - 1;
                    }
                }
            } else {
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

                        if (backPackContents) {
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

                if ((temp > 0) && backPackContents) {
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
    }

    return temp;
}


int Character::increaseAtPos(unsigned char pos, int count) {
    int temp = count;

    if ((pos > 0) && (pos < MAX_BELT_SLOTS + MAX_BODY_ITEMS)) {
        if (weightOK(characterItems[ pos ].getId(), count, nullptr)) {

            temp = characterItems[ pos ].getNumber() + count;
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
    if ((pos > 0) && (pos < MAX_BELT_SLOTS + MAX_BODY_ITEMS)) {
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
    for (unsigned char i = 0; i < MAX_BELT_SLOTS + MAX_BODY_ITEMS; ++i) {
        auto itemId = characterItems[i].getId();

        if (itemId != 0) {
            const auto &tempCommon = Data::CommonItems[characterItems[ i ].getId()];

            if (tempCommon.isValid() && tempCommon.rotsInInventory) {
                if (!characterItems[i].survivesAgeing()) {
                    if (itemId != tempCommon.ObjectAfterRot) {
                        characterItems[i].setId(tempCommon.ObjectAfterRot);

                        const auto &afterRotCommon = Data::CommonItems[tempCommon.ObjectAfterRot];

                        if (afterRotCommon.isValid()) {
                            characterItems[i].setWear(afterRotCommon.AgeingSpeed);
                        }
                    } else {
                        characterItems[i ].reset();
                    }
                }
            }
        }
    }

    if ((characterItems[ BACKPACK ].getId() != 0) && backPackContents) {
        backPackContents->doAge(true);
    }

    for (const auto &depot : depotContents) {
        if (depot.second) {
            depot.second->doAge(true);
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
    if (target && target->IsAlive()) {
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
                ServerCommandPointer cmd(new BBSendActionTC(id, name, 1 , "Attacks : " + target->to_string()));
                _world->monitoringClientList->sendCommand(cmd);
            } else {
                ServerCommandPointer cmd(new BBSendActionTC(id, name, 1 , "Killed : " + target->to_string()));
                _world->monitoringClientList->sendCommand(cmd);
            }
        }

        if (!target->IsAlive()) {
            // target was killed...
            Logger::info(LogFacility::Player) << *this << " killed " << *target << Log::end;
        }

        return (target->IsAlive());
    }

    return false;
}

std::string Character::getSkillName(TYPE_OF_SKILL_ID s) const {
    if (Data::Skills.exists(s)) {
        return Data::Skills[s].englishName;
    } else {
        return "unknown skill";
    }
}

unsigned short int Character::getSkill(TYPE_OF_SKILL_ID s) const {
    auto iterator = skills.find(s);

    if (iterator == skills.end()) {
        return 0;
    } else {
        return (*iterator).second.major;
    }
}

unsigned short int Character::getMinorSkill(TYPE_OF_SKILL_ID s) const {
    auto iterator = skills.find(s);

    if (iterator == skills.end()) {
        return 0;
    } else {
        return (*iterator).second.minor;
    }
}


void Character::setSkinColor(uint8_t red, uint8_t green, uint8_t blue) {
    _appearance.skin.red = red;
    _appearance.skin.green = green;
    _appearance.skin.blue = blue;
    updateAppearanceForAll(true);
}


void Character::getSkinColor(uint8_t &red, uint8_t &green, uint8_t &blue) const {
    red = _appearance.skin.red;
    green =_appearance.skin.green;
    blue = _appearance.skin.blue;
}


void Character::setHairColor(uint8_t red, uint8_t green, uint8_t blue) {
    _appearance.hair.red = red;
    _appearance.hair.green = green;
    _appearance.hair.blue = blue;
    updateAppearanceForAll(true);
}


void Character::getHairColor(uint8_t &red, uint8_t &green, uint8_t &blue) const {
    red = _appearance.hair.red;
    green = _appearance.hair.green;
    blue = _appearance.hair.blue;
}


void Character::setHair(uint8_t hairID) {
    _appearance.hairtype = hairID;
    updateAppearanceForAll(true);
}


uint8_t Character::getHair() const {
    return _appearance.hairtype;
}


void Character::setBeard(uint8_t beardID) {
    _appearance.beardtype = beardID;
    updateAppearanceForAll(true);
}


uint8_t Character::getBeard() const {
    return _appearance.beardtype;
}

void Character::setAttribute(Character::attributeIndex attribute, Attribute::attribute_t value) {
    auto &attrib = attributes[attribute];
    auto oldValue = attrib.getValue();
    attrib.setValue(value);
    auto newValue = attrib.getValue();

    if (newValue != oldValue) {
        handleAttributeChange(attribute);
    }
}

Attribute::attribute_t Character::getAttribute(Character::attributeIndex attribute) const {
    return attributes[attribute].getValue();
}

Attribute::attribute_t Character::increaseAttribute(Character::attributeIndex attribute, int amount) {
    auto &attrib = attributes[attribute];
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

void Character::setAttrib(const std::string &name, Attribute::attribute_t value) {
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

Attribute::attribute_t Character::increaseAttrib(const std::string &name, int amount) {
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
    if (!Data::Skills.exists(skill)) {
        return 0;
    }

    auto iterator = skills.find(skill);
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
    if (!Data::Skills.exists(skill)) {
        return 0;
    }

    auto iterator = skills.find(skill);

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
    if (!Data::Skills.exists(skill)) {
        return 0;
    }

    auto iterator = skills.find(skill);

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

auto Character::getSkillValue(TYPE_OF_SKILL_ID s) const -> const skillvalue * {
    auto it = skills.find(s);

    if (it == skills.end()) {
        return nullptr;
    } else {
        return &(it->second);
    }
}

void Character::learn(TYPE_OF_SKILL_ID skill, uint32_t actionPoints, uint8_t opponent) {
    if (!Data::Skills.exists(skill)) {
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


bool Character::isInRange(Character *cc, unsigned short int distancemetric) const {
    if (cc) {
        short int pz = cc->pos.z - pos.z;
        short int px = cc->pos.x - pos.x;
        short int py = cc->pos.y - pos.y;

        if (((abs(px) + abs(py)) <= distancemetric) && (pz==0)) {
            return true;
        }
    }

    return false;
}

unsigned short int Character::getScreenRange() const {
    return 14;
}

bool Character::isInRangeToField(const position &m_pos, unsigned short int distancemetric) const {
    short int pz = m_pos.z - pos.z;
    short int px = m_pos.x - pos.x;
    short int py = m_pos.y - pos.y;

    if (((abs(px) + abs(py)) <= distancemetric) && (pz == 0)) {
        return true;
    } else {
        return false;
    }
}

unsigned short int Character::distanceMetricToPosition(const position &m_pos) const {
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

unsigned short int Character::distanceMetric(Character *cc) const {
    unsigned short int ret=0xFFFF;

    if (cc) {
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


unsigned short int Character::maxLoadWeight() const {
    return getAttribute(Character::strength) * 500 + 5000;
}


int Character::LoadWeight() const {
    int load=0;

    // alle Items bis auf den Rucksack
    for (int i=1; i < MAX_BODY_ITEMS + MAX_BELT_SLOTS; ++i) {
        load += characterItems[i].getWeight();
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


bool Character::weightOK(TYPE_OF_ITEM_ID id, int count, Container *tcont) const {
    bool ok;

    int realweight = LoadWeight();

    if (tcont) {
        ok = (realweight + weightContainer(id, 1, tcont)) <= maxLoadWeight();
    } else {
        const auto &common = Data::CommonItems[id];
        ok = (realweight + common.Weight * count) <= maxLoadWeight();
    }

    return ok;
}


int Character::weightContainer(TYPE_OF_ITEM_ID id, int count, Container *tcont) const {
    int temp=0;

    if (id != 0) {
        const auto &tempCommon = Data::CommonItems[id];

        if (tempCommon.isValid()) {
            if (count > 0) {
                temp = tempCommon.Weight;
            } else {
                temp = 0 - tempCommon.Weight;
            }
        }

        if (tcont) {
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

Character::movement_type Character::GetMovement() const {
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

std::string Character::alterSpokenMessage(const std::string &message, int languageSkill) const {
    int counter=0;
    std::string alteredMessage;

    alteredMessage=message;

    while (message[counter]!=0) {
        if (Random::uniform(0,70)>languageSkill) {
            alteredMessage[counter]='*';
        }

        counter++;
    }

    return alteredMessage;
}

int Character::getLanguageSkill(int languageSkillNumber) const {
    return 100;
}

void Character::talk(talk_type tt, const std::string &message) { //only for say, whisper, shout
    talk(tt, message, message);

    if (character == player) {
#ifdef LOG_TALK
        std::string talkType;

        switch (tt) {
        case tt_say:
            talkType = "says";
            break;

        case tt_whisper:
            talkType = "whispers";
            break;

        case tt_yell:
            talkType = "shouts";
            break;
        }

        Logger::info(LogFacility::Chat) << *this << " " << talkType << ": " << message << Log::end;
#endif
        ServerCommandPointer cmd(new BBTalkTC(id ,name, static_cast<unsigned char>(tt), message));
        _world->monitoringClientList->sendCommand(cmd);
    }
}

void Character::talk(talk_type tt, const std::string &german, const std::string &english) { //only for say, whisper, shout
    uint16_t cost = 0;
    lastSpokenText = english;

    switch (tt) {
    case tt_say:

        if (!IsAlive()) {
            return;
        }

        cost = P_SAY_COST;
        break;

    case tt_whisper:
        cost = P_WHISPER_COST;
        break;

    case tt_yell:

        if (!IsAlive()) {
            return;
        }

        cost = P_SHOUT_COST;
        break;
    }

    _world->sendMessageToAllCharsInRange(german, english, tt, this);
    actionPoints -= cost;
}

void Character::turn(direction dir) {
    if (dir != dir_up && dir != dir_down && dir != static_cast<direction>(faceto)) {
        faceto = (Character::face_to)dir;
        _world->sendSpinToAllVisiblePlayers(this);
    }
}

void Character::turn(const position &posi) {
    //attack the player which we have found
    short int xoffs = posi.x - pos.x;
    short int yoffs = posi.y - pos.y;

    if (abs(xoffs)>abs(yoffs)) {
        turn(static_cast<direction>((xoffs>0)?2:6));
    } else {
        turn(static_cast<direction>((yoffs>0)?4:0));
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
    newpos.move(dir);

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

bool Character::moveToPossible(const Field *field) const {
    // for monsters/npcs we just use the field infos for now
    return field->moveToPossible();
}

uint16_t Character::getMovementCost(const Field *sourcefield) const {
    uint16_t walkcost = 0;

    auto tileId = sourcefield->getTileId();
    const auto &primaryTile = Data::Tiles[tileId];
    uint16_t tileWalkingCost = primaryTile.walkingCost;

    tileId = sourcefield->getSecondaryTileId();
    const auto &secondaryTile = Data::Tiles[tileId];
    uint16_t secondaryWalkingCost = secondaryTile.walkingCost;

    if (secondaryWalkingCost < tileWalkingCost) {
        tileWalkingCost = secondaryWalkingCost;
    }

    switch (_movement) {
    case walk:
        walkcost += tileWalkingCost;
        break;

    case fly: // walking cost independent of source field
        walkcost += NP_STANDARDFLYCOST;
        break;

    case crawl: // just double the ap necessary for walking
        walkcost += 2 * tileWalkingCost;
        break;
    }

    if (character != player) {
        walkcost += STANDARD_MONSTER_WALKING_COST;
    }

    walkcost = (walkcost * P_MOVECOSTFORMULA_walkingCost_MULTIPLIER) / (getAttribute(Character::agility) + P_MOVECOSTFORMULA_agility_ADD);

    return walkcost;
}

void Character::updatePos(const position &newpos) {
    pos = newpos;
}

void Character::receiveText(talk_type tt, const std::string &message, Character *cc) {
    // overloaded where necessary
}

void Character::introducePlayer(Player *) {
    // overloaded in Player
}

void Character::teachMagic(unsigned char type, unsigned char flag) {
    // overloaded in Player
}

bool Character::Warp(const position &targetPos) {
    position oldpos = pos;
    Field *fold = nullptr;

    if (_world->GetPToCFieldAt(fold, pos.x, pos.y, pos.z)) {
        Field *fnew = nullptr;
        position newPos = targetPos;

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

bool Character::forceWarp(const position &newPos) {
    position oldpos = pos;
    Field *fold = nullptr;

    if (_world->GetPToCFieldAt(fold, pos.x, pos.y, pos.z)) {
        Field *fnew = nullptr;

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

void Character::startMusic(short int title) {
    //Nothing to do here, overloaded for players
}

void Character::defaultMusic() {
    //Nothing to do here, overloaded for players
}

void Character::inform(const std::string &text, informType type) const {
    // override for char types that need this kind of information
}

void Character::inform(const std::string &, const std::string &, informType type) const {
    // override for char types that need this kind of information
}

void Character::changeQualityItem(TYPE_OF_ITEM_ID id, short int amount) {
    if ((characterItems[ BACKPACK ].getId() != 0) && backPackContents) {
        if (backPackContents->changeQuality(id, amount)) {
            return;
        }
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
    if (pos < MAX_BODY_ITEMS + MAX_BELT_SLOTS) {
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
        } else {

            if (pos == RIGHT_TOOL && characterItems[LEFT_TOOL].getId() == BLOCKEDITEM) {
                characterItems[LEFT_TOOL].reset();
            } else if (pos == LEFT_TOOL && characterItems[RIGHT_TOOL].getId() == BLOCKEDITEM) {
                characterItems[RIGHT_TOOL].reset();
            }

            characterItems[ pos ].reset();
            return;
        }
    }
}

void Character::callAttackScript(Character *Attacker, Character *Defender) {
    const auto weaponId = characterItems[RIGHT_TOOL].getId();

    if (weaponId != 0) {
        if (Data::WeaponItems.exists(weaponId)) {
            const auto &script = Data::WeaponItems.script(weaponId);

            if (script && script->existsEntrypoint("onAttack")) {
                script->onAttack(Attacker, Defender);
            }
        }
    }

    standardFightingScript->onAttack(Attacker, Defender);
}

void Character::setQuestProgress(TYPE_OF_QUEST_ID questid, TYPE_OF_QUESTSTATUS progress) {
    // Nothing to do here, overridden for players
}

TYPE_OF_QUESTSTATUS Character::getQuestProgress(TYPE_OF_QUEST_ID questid, int &time) const {
    // Nothing to do here, overridden for players
    return 0;
}

std::vector<ScriptItem> Character::getItemList(TYPE_OF_ITEM_ID id) const {
    std::vector<ScriptItem> list;

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
            item.owner = const_cast<Character *>(this);
            list.push_back(item);
        }
    }

    if ((characterItems[ BACKPACK ].getId() != 0) && backPackContents) {
        backPackContents->addContentToList(id, list);
    }

    return list;
}


Container *Character::GetBackPack() const {
    return backPackContents;
}

Container *Character::GetDepot(uint32_t depotid) const {
    auto it = depotContents.find(depotid + 1);;

    if (it == depotContents.end()) {
        return nullptr;
    } else {
        return it->second;
    }
}


uint32_t Character::idleTime() const {
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
        ServerCommandPointer cmd(new AppearanceTC(this, target));
        target->sendCharAppearance(id, cmd, always);
    }
}

void Character::updateAppearanceForAll(bool always) {
    if (!isinvisible) {
        for (const auto &player : World::get()->Players.findAllCharactersInScreen(pos)) {
            ServerCommandPointer cmd(new AppearanceTC(this, player));
            player->sendCharAppearance(id, cmd, always);
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
        ServerCommandPointer cmd(new AnimationTC(id, animID));

        for (const auto &player : World::get()->Players.findAllCharactersInScreen(pos)) {
            player->Connection->addCommand(cmd);
        }
    }
}

bool Character::pageGM(const std::string &ticket) {
    //Nothing to do here, overloaded in Player
    return false;
}

void Character::setId(TYPE_OF_CHARACTER_ID id) {
    this->id = id;
}

void Character::setName(const std::string &name) {
    this->name = name;
}

std::ostream &operator<<(std::ostream &os, const Character &character) {
    return os << character.to_string();
};
