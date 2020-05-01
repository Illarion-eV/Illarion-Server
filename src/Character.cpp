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

#include "Character.hpp"

#include <map>
#include <algorithm>
#include <cmath>

#include "character_ptr.hpp"
#include "a_star.hpp"
#include "Container.hpp"
#include "Player.hpp"
#include "Random.hpp"
#include "World.hpp"
#include "Field.hpp"
#include "Logger.hpp"
#include "MonitoringClients.hpp"
#include "LongTimeAction.hpp"

#include "data/Data.hpp"
#include "data/RaceTypeTable.hpp"
#include "data/TilesTable.hpp"

#include "script/LuaWeaponScript.hpp"
#include "script/LuaLearnScript.hpp"

#include "netinterface/protocol/BBIWIServerCommands.hpp"
#include "netinterface/protocol/ServerCommands.hpp"

#define MAJOR_SKILL_GAP 100

extern std::shared_ptr<LuaLearnScript>learnScript;
extern std::shared_ptr<LuaWeaponScript> standardFightingScript;
extern std::unique_ptr<RaceTypeTable> raceTypes;

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


Character::Character(const appearance &appearance) : effects(this), waypoints(this), _world(World::get()), _appearance(appearance), attributes(ATTRIBUTECOUNT) {
    setAlive(true);

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

    backPackContents = nullptr;

    magic.type = MAGE;
    magic.flags[ MAGE ] = 0x00000000;
    magic.flags[ PRIEST ] = 0x00000000;
    magic.flags[ BARD ] = 0x00000000;
    magic.flags[ DRUID ] = 0x00000000;
}

Character::~Character() {
    if (backPackContents) {
        delete backPackContents;
        backPackContents = nullptr;
    }

    std::for_each(depotContents.rbegin(), depotContents.rend(),
                  [](const decltype(depotContents)::value_type &value) {
        delete value.second;
    });
}

TYPE_OF_CHARACTER_ID Character::getId() const {
    return id;
}

const std::string &Character::getName() const {
    return name;
}

short int Character::getActionPoints() const {
    return actionPoints;
}

short int Character::getMinActionPoints() const {
    return NP_MIN_AP;
}

short int Character::getMaxActionPoints() const {
    return NP_MAX_AP;
}

void Character::setActionPoints(short int ap) {
    if (ap > getMaxActionPoints()) {
        actionPoints = getMaxActionPoints();
    } else {
        actionPoints = ap;
    }
}

void Character::increaseActionPoints(short int ap) {
    setActionPoints(actionPoints + ap);
}

bool Character::canAct() const {
    return actionPoints >= getMaxActionPoints();
}

short int Character::getFightPoints() const {
    return fightPoints;
}

short int Character::getMinFightPoints() const {
    return NP_MIN_FP;
}

short int Character::getMaxFightPoints() const {
    return NP_MAX_FP;
}

void Character::setFightPoints(short int fp) {
    if (fp > getMaxFightPoints()) {
        fightPoints = getMaxFightPoints();
    } else {
        fightPoints = fp;
    }
}

void Character::increaseFightPoints(short int fp) {
    setFightPoints(fightPoints + fp);
}

bool Character::canFight() const {
    return fightPoints >= getMinFightPoints();
}

short int Character::getActiveLanguage() const {
    return activeLanguage;
}

void Character::setActiveLanguage(short int l) {
    activeLanguage = l;
}

const position &Character::getPosition() const {
    return pos;
}

bool Character::getAttackMode() const {
    return attackmode;
}

void Character::setAttackMode(bool attack) {
    attackmode = attack;
}

const std::string &Character::getLastSpokenText() const {
    return lastSpokenText;
}

bool Character::isInvisible() const {
    return isinvisible;
}

void Character::setInvisible(bool invisible) {
    isinvisible = invisible;
}

int Character::countItem(TYPE_OF_ITEM_ID itemid) const {
    int temp = 0;

    for (const auto &item: items) {
        if (item.getId() == itemid) {
            temp += item.getNumber();
        }
    }

    if ((items[ BACKPACK ].getId() != 0) && backPackContents) {
        temp += backPackContents->countItem(itemid);
    }

    return temp;
}

int Character::countItemAt(const std::string &where, TYPE_OF_ITEM_ID itemid, script_data_exchangemap const *data) const {
    int temp = 0;

    if (where == "all") {
        for (const auto &item: items) {
            if (item.getId() == itemid &&
                (data == nullptr || item.hasData(*data))) {
                temp += item.getNumber();
            }
        }

        if ((items[ BACKPACK ].getId() != 0) && backPackContents) {
            temp += backPackContents->countItem(itemid, data);
        }

        return temp;
    }

    if (where == "belt") {
        for (unsigned char i = MAX_BODY_ITEMS; i < MAX_BODY_ITEMS + MAX_BELT_SLOTS; ++i) {
            if (items[ i ].getId() == itemid && (data == nullptr || items[i].hasData(*data))) {
                temp += items[ i ].getNumber();
            }
        }

        return temp;
    }

    if (where == "body") {
        for (unsigned char i = 0; i < MAX_BODY_ITEMS; ++i) {
            if (items[ i ].getId() == itemid && (data == nullptr || items[i].hasData(*data))) {
                temp += items[ i ].getNumber();
            }
        }

        return temp;
    }

    if (where == "backpack") {
        if ((items[ BACKPACK ].getId() != 0) && backPackContents) {
            temp += backPackContents->countItem(itemid , data);
        }

        return temp;
    }

    return temp;
}

ScriptItem Character::GetItemAt(unsigned char itempos) {
    ScriptItem item;

    if (itempos < MAX_BODY_ITEMS + MAX_BELT_SLOTS) {
        item = static_cast<ScriptItem>(items[ itempos ]);
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

    if ((items[ BACKPACK ].getId() != 0) && backPackContents) {
        temp = backPackContents->eraseItem(itemid, temp, data);
    }

    if (temp > 0) {
        // BACKPACK als Item erstmal auslassen
        for (unsigned char i = MAX_BELT_SLOTS + MAX_BODY_ITEMS - 1; i > 0; --i) {
            if ((items[ i ].getId() == itemid && (data == nullptr || items[ i ].hasData(*data))) && (temp > 0)) {
                if (temp >= items[ i ].getNumber()) {
                    temp = temp - items[ i ].getNumber();
                    items[ i ].reset();
                } else {
                    items[ i ].setNumber(items[ i ].getNumber() - temp);
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
        const auto &cos = Data::Items[newid];

        if (cos.isValid()) {
            if (!Data::ContainerItems.exists(newid)) {
                if (items[ pos ].getId() == 0) {
                    if (temp > cos.MaxStack) {
                        items[ pos ].setId(newid);
                        items[ pos ].setWear(cos.AgeingSpeed);
                        items[ pos ].setNumber(cos.MaxStack);
                        temp -= cos.MaxStack;
                    } else {
                        items[ pos ].setId(newid);
                        items[ pos ].setWear(cos.AgeingSpeed);
                        items[ pos ].setNumber(temp);
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
        const auto &cos = Data::Items[id];

        if (cos.isValid()) {
            if (Data::ContainerItems.exists(id)) {
                if (items[ BACKPACK ].getId() == 0) {
                    items[ BACKPACK ].setId(id);
                    items[ BACKPACK ].setWear(cos.AgeingSpeed);
                    items[ BACKPACK ].setQuality(quality);
                    items[ BACKPACK ].setData(data);
                    items[ BACKPACK ].setNumber(1);
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
                        --temp;
                    }
                }
            } else {
                int old_temp = temp;

                if (cos.MaxStack > 1) {
                    if (backPackContents) {
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

                            auto leftOver = backPackContents->mergeItem(it);

                            if (leftOver > 0) {
                                ok = false;
                            }

                            temp -= it.getNumber() - leftOver;
                        }
                    }

                    for (unsigned char i = MAX_BODY_ITEMS; i < MAX_BELT_SLOTS + MAX_BODY_ITEMS && temp > 0; ++i) {
                        if ((items[ i ].getId() == id) && (items[ i ].equalData(data))) {
                            int itemsToCreate = temp;
                            temp = items[ i ].increaseNumberBy(temp);

                            if (itemsToCreate != temp) {
                                items[ i ].setWear(cos.AgeingSpeed);
                                items[ i ].setQuality(quality);
                            }
                        }
                    }
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
                
                for (unsigned char i = MAX_BODY_ITEMS; i < MAX_BELT_SLOTS + MAX_BODY_ITEMS && temp > 0; ++i) {
                    if (items[ i ].getId() == 0) {
                        items[ i ].setId(id);
                        items[ i ].setWear(cos.AgeingSpeed);
                        items[ i ].setQuality(quality);
                        items[ i ].setData(data);
                        temp = items[ i ].increaseNumberBy(temp);
                    }
                }

                if (temp != old_temp && cos.Brightness > 0) {
                    updateAppearanceForAll(true);
                }
            }
        }
    }

    return temp;
}


int Character::increaseAtPos(unsigned char pos, int count) {
    int temp = count;

    if ((pos > 0) && (pos < MAX_BELT_SLOTS + MAX_BODY_ITEMS)) {
        if (weightOK(items[ pos ].getId(), count, nullptr)) {

            temp = items[ pos ].getNumber() + count;
            auto maxStack = items[pos].getMaxStack();

            if (temp > maxStack) {
                items[ pos ].setNumber(maxStack);
                temp = temp - maxStack;
            } else if (temp <= 0) {
                bool updateBrightness = World::get()->getItemStatsFromId(items[ pos ].getId()).Brightness > 0;
                temp = count + items[ pos ].getNumber();
                items[ pos ].reset();

                if (updateBrightness) {
                    updateAppearanceForAll(true);
                }
            } else {
                items[ pos ].setNumber(temp);
                temp = 0;
            }
        }
    }

    return temp;
}


bool Character::swapAtPos(unsigned char pos, TYPE_OF_ITEM_ID newid, uint16_t newQuality) {
    if ((pos > 0) && (pos < MAX_BELT_SLOTS + MAX_BODY_ITEMS)) {
        bool updateBrightness = World::get()->getItemStatsFromId(items[ pos ].getId()).Brightness > 0 || World::get()->getItemStatsFromId(newid).Brightness > 0;
        items[ pos ].setId(newid);

        if (updateBrightness) {
            updateAppearanceForAll(true);
        }

        if (newQuality > 0) {
            items[ pos ].setQuality(newQuality);
        }

        return true;
    } else {
        return false;
    }
}


void Character::ageInventory() {
    for (auto &item: items) {
        auto itemId = item.getId();

        if (itemId != 0) {
            const auto &itemStruct = Data::Items[itemId];

            if (itemStruct.isValid() && itemStruct.rotsInInventory) {
                if (!item.survivesAgeing()) {
                    if (itemId != itemStruct.ObjectAfterRot) {
                        item.setId(itemStruct.ObjectAfterRot);

                        const auto &afterRotItemStruct = Data::Items[itemStruct.ObjectAfterRot];

                        if (afterRotItemStruct.isValid()) {
                            item.setWear(afterRotItemStruct.AgeingSpeed);
                        }
                    } else {
                        item.reset();
                    }
                }
            }
        }
    }

    if ((items[ BACKPACK ].getId() != 0) && backPackContents) {
        backPackContents->doAge(true);
    }

    for (const auto &depot : depotContents) {
        if (depot.second) {
            depot.second->doAge(true);
        }
    }
}

void Character::setAlive(bool t) {
    alive = t;
}


bool Character::attack(Character *target) {
    if (target && target->isAlive()) {
        if (!actionRunning()) {
            if (target->isAlive()) {
                if (target->getType() == player) {
                    Player *pl = dynamic_cast<Player *>(target);
                    pl->ltAction->actionDisturbed(this);
                }

                callAttackScript(this, target);
            }
        }

        if (getType() == player) {
            if (target->isAlive()) {
                ServerCommandPointer cmd = std::make_shared<BBSendActionTC>(id, 1 , "Attacks : " + target->to_string());
                _world->monitoringClientList->sendCommand(cmd);
            } else {
                ServerCommandPointer cmd = std::make_shared<BBSendActionTC>(id, 1 , "Killed : " + target->to_string());
                _world->monitoringClientList->sendCommand(cmd);
            }
        }

        if (!target->isAlive()) {
            // target was killed...
            Logger::info(LogFacility::Player) << *this << " killed " << *target << Log::end;
        }

        return (target->isAlive());
    }

    return false;
}

void Character::stopAttack() {
    setAttackMode(false);
    enemyid = 0;
}

character_ptr Character::getAttackTarget() const {
    auto target = _world->findCharacter(enemyid);
    return character_ptr(target);
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


void Character::setSkinColour(const Colour &c) {
    _appearance.skin = c;
    updateAppearanceForAll(true);
}


Colour Character::getSkinColour() const {
    return _appearance.skin;
}


void Character::setHairColour(const Colour &c) {
    _appearance.hair = c;
    updateAppearanceForAll(true);
}


Colour Character::getHairColour() const {
    return _appearance.hair;
}


void Character::setHair(uint8_t hairID) {
    if (raceTypes->isHairAvailable(race, getAttribute(sex), hairID)) {
        _appearance.hairtype = hairID;
        updateAppearanceForAll(true);
    } else {
        Logger::error(LogFacility::Script) << "Race " << race << " subtype "
                << getAttribute(sex) << " has no hair with id "
                << int(hairID) << ". Leaving hair unchanged at "
                << int(_appearance.hairtype) << "." << Log::end;
    }
}


uint8_t Character::getHair() const {
    return _appearance.hairtype;
}


void Character::setBeard(uint8_t beardID) {
    if (raceTypes->isBeardAvailable(race, getAttribute(sex), beardID)) {
        _appearance.beardtype = beardID;
        updateAppearanceForAll(true);
    } else {
        Logger::error(LogFacility::Script) << "Race " << race << " subtype "
                << getAttribute(sex) << " has no beard with id "
                << int(beardID) << ". Leaving beard unchanged at "
                << int(_appearance.beardtype) << "." << Log::end;
    }
}


uint8_t Character::getBeard() const {
    return _appearance.beardtype;
}

bool Character::setBaseAttribute(Character::attributeIndex attribute, Attribute::attribute_t value) {
    auto &attrib = attributes[attribute];
    auto oldValue = attrib.getValue();

    if (isBaseAttributeValid(attribute, value)) {
        attrib.setBaseValue(value);
        auto newValue = attrib.getValue();

        if (newValue != oldValue) {
            handleAttributeChange(attribute);
        }

        return true;
    } else {
        return false;
    }
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

Attribute::attribute_t Character::getBaseAttribute(Character::attributeIndex attribute) const {
    return attributes[attribute].getBaseValue();
}

Attribute::attribute_t Character::getAttribute(Character::attributeIndex attribute) const {
    return attributes[attribute].getValue();
}

bool Character::increaseBaseAttribute(Character::attributeIndex attribute, int amount) {
    auto &attrib = attributes[attribute];
    auto oldValue = attrib.getValue();
    
    if (isBaseAttributeValid(attribute, attrib.getBaseValue() + amount)) {
        attrib.increaseBaseValue(amount);
        auto newValue = attrib.getValue();

        if (newValue != oldValue) {
            handleAttributeChange(attribute);
        }

        return true;
    } else {
        return false;
    }
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

bool Character::isBaseAttributeValid(Character::attributeIndex attribute, Attribute::attribute_t value) const {
    return Data::Races.isBaseAttributeInLimits(getRace(), attribute, value);
}

uint16_t Character::getBaseAttributeSum() const {
    return attributes[Character::agility].getBaseValue()
        + attributes[Character::constitution].getBaseValue()
        + attributes[Character::dexterity].getBaseValue()
        + attributes[Character::essence].getBaseValue()
        + attributes[Character::intelligence].getBaseValue()
        + attributes[Character::perception].getBaseValue()
        + attributes[Character::strength].getBaseValue()
        + attributes[Character::willpower].getBaseValue();
}

uint16_t Character::getMaxAttributePoints() const {
    return Data::Races.getMaxAttributePoints(getRace());
}

bool Character::saveBaseAttributes() {
    return false;
}

void Character::handleAttributeChange(Character::attributeIndex attribute) {
    if (attribute == Character::hitpoints) {
        setAlive(getAttribute(hitpoints) > 0);
        _world->sendHealthToAllVisiblePlayers(this, getAttribute(hitpoints));
    }
}

bool Character::isBaseAttribValid(const std::string &name, Attribute::attribute_t value) const {
    try {
        Character::attributeIndex attribute = attributeMap.at(name);
        return isBaseAttributeValid(attribute, value);
    } catch (...) {
        return false;
    }
}

bool Character::setBaseAttrib(const std::string &name, Attribute::attribute_t value) {
    try {
        Character::attributeIndex attribute = attributeMap.at(name);
        return setBaseAttribute(attribute, value);
    } catch (...) {
        return false;
    }
}

void Character::setAttrib(const std::string &name, Attribute::attribute_t value) {
    try {
        Character::attributeIndex attribute = attributeMap.at(name);
        setAttribute(attribute, value);
    } catch (...) {

    }
}

Attribute::attribute_t Character::getBaseAttrib(const std::string &name) {
    try {
        Character::attributeIndex attribute = attributeMap.at(name);
        return getBaseAttribute(attribute);
    } catch (...) {
        return 0;
    }
}



bool Character::increaseBaseAttrib(const std::string &name, int amount) {
    try {
        Character::attributeIndex attribute = attributeMap.at(name);
        return increaseBaseAttribute(attribute, amount);
    } catch (...) {
        return false;
    }
}


Attribute::attribute_t Character::increaseAttrib(const std::string &name, int amount) {
    if (name == "sex") {
        return getAttribute(Character::sex);
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
    }
}


void Character::deleteAllSkills() {
    skills.clear();
}


bool Character::isInRange(Character *cc, unsigned short int distancemetric) const {
    if (cc) {
        return isInRangeToField(cc->pos, distancemetric);
    }

    return false;
}

bool Character::isInScreen(const position &pos) const {
    short int dx = abs(this->pos.x - pos.x);
    short int dy = abs(this->pos.y - pos.y);
    short int dz = abs(this->pos.z - pos.z);

    return dx + dy <= getScreenRange() && -RANGEDOWN <= dz && dz <= RANGEUP;
}

unsigned short int Character::getScreenRange() const {
    return 14;
}

bool Character::isInRangeToField(const position &m_pos, unsigned short int distancemetric) const {
    short int dz = abs(m_pos.z - pos.z);
    short int dx = abs(m_pos.x - pos.x);
    short int dy = abs(m_pos.y - pos.y);

    return dx <= distancemetric && dy <= distancemetric && dz == 0;
}

unsigned short int Character::distanceMetricToPosition(const position &m_pos) const {
    short int dz = abs(pos.z - m_pos.z);
    short int dx = abs(pos.x - m_pos.x);
    short int dy = abs(pos.y - m_pos.y);

    return std::max(dx, std::max(dy, dz));
}

unsigned short int Character::distanceMetric(Character *cc) const {
    if (cc) {
        return distanceMetricToPosition(cc->pos);
    } else {
        return 0xFFFF;
    }
}


unsigned short int Character::maxLoadWeight() const {
    return getAttribute(Character::strength) * 500 + 5000;
}


int Character::LoadWeight() const {
    int load=0;

    // alle Items bis auf den Rucksack
    for (int i=1; i < MAX_BODY_ITEMS + MAX_BELT_SLOTS; ++i) {
        load += items[i].getWeight();
    }

    // Rucksack
    load += weightContainer(items[0].getId(), 1, backPackContents);

    if (load > 30000) {
        return 30000;
    } else if (load < 0) {
        return 0;
    } else {
        return load;
    }
}


float Character::relativeLoad() const {
    return float(LoadWeight()) / maxLoadWeight();
}


auto Character::loadFactor() const -> LoadLevel {
    auto load = relativeLoad();

    if (load > 1.0) {
        return LoadLevel::overtaxed;
    } else if (load > 0.75) {
        return LoadLevel::burdened;
    }

    return LoadLevel::unburdened;
}


bool Character::weightOK(TYPE_OF_ITEM_ID id, int count, Container *tcont) const {
    if (count < 0) {
        return true;
    }

    int realweight = LoadWeight();

    if (tcont) {
        return (realweight + weightContainer(id, 1, tcont)) <= maxLoadWeight();
    } else {
        const auto &itemStruct = Data::Items[id];
        return (realweight + itemStruct.Weight * count) <= maxLoadWeight();
    }
}


int Character::weightContainer(TYPE_OF_ITEM_ID id, int count, Container *tcont) const {
    int temp=0;

    if (id != 0) {
        const auto &itemStruct = Data::Items[id];

        if (itemStruct.isValid()) {
            if (count > 0) {
                temp = itemStruct.Weight;
            } else {
                temp = 0 - itemStruct.Weight;
            }
        }

        if (tcont) {
            try {
                if (count > 0) {
                    temp += tcont->weight();
                } else {
                    temp -= tcont->weight();
                }
            } catch (RecursionException &e) {
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

movement_type Character::GetMovement() const {
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

    if (getType() == player) {
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
        ServerCommandPointer cmd = std::make_shared<BBTalkTC>(id, static_cast<unsigned char>(tt), message);
        _world->monitoringClientList->sendCommand(cmd);
    }
}

void Character::talk(talk_type tt, const std::string &german, const std::string &english) { //only for say, whisper, shout
    uint16_t cost = 0;
    lastSpokenText = english;

    switch (tt) {
    case tt_say:

        if (!isAlive()) {
            return;
        }

        cost = P_SAY_COST;
        break;

    case tt_whisper:
        cost = P_WHISPER_COST;
        break;

    case tt_yell:

        if (!isAlive()) {
            return;
        }

        cost = P_SHOUT_COST;
        break;
    }

    _world->sendMessageToAllCharsInRange(german, english, tt, this);
    actionPoints -= cost;
}

void Character::turn(direction dir) {
    if (dir < 8 && dir != static_cast<direction>(faceto)) {
        faceto = (Character::face_to)dir;
        _world->sendSpinToAllVisiblePlayers(this);
    }
}

void Character::turn(const position &posi) {
    short int xoffs = posi.x - pos.x;
    short int yoffs = posi.y - pos.y;

    if (abs(xoffs)>abs(yoffs)) {
        turn(static_cast<direction>((xoffs>0)?2:6));
    } else {
        turn(static_cast<direction>((yoffs>0)?4:0));
    }
}

bool Character::move(direction dir, bool active) {
    _world->TriggerFieldMove(this,false);

    // if we move we look into that direction...
    if (dir != dir_up && dir != dir_down) {
        faceto = (Character::face_to)dir;
    }

    // check if we can move to our target field
    position newpos = pos;
    newpos.move(dir);

    try {

        Field &oldField = _world->fieldAt(pos);
        Field &newField = _world->fieldAtOrBelow(newpos);

        if (moveToPossible(newField)) {
            bool diagonalMove = pos.x != newpos.x && pos.y != newpos.y;
            uint16_t movementcost = getMoveTime(newField, diagonalMove, false);
            actionPoints -= movementcost/100;

            oldField.removeChar();
            newField.setChar();

            setPosition(newpos);

            if (active) {
                _world->sendCharacterMoveToAllVisibleChars(this, movementcost);
            } else {
                _world->sendPassiveMoveToAllVisiblePlayers(this);
            }

            // check if there are teleporters or other special flags on this field
            _world->checkFieldAfterMove(this, newField);

            _world->TriggerFieldMove(this, true);

            return true;
        }
    } catch (FieldNotFound &) {
    }

    return false;
}

bool Character::moveToPossible(const Field &field) const {
    // for monsters/npcs we just use the field infos for now
    return field.moveToPossible();
}

TYPE_OF_WALKINGCOST Character::getMoveTime(const Field &targetField, bool diagonalMove, bool running) const {
    static const float sqrt2 = std::sqrt(2.0);
    TYPE_OF_WALKINGCOST walkcost;

    switch (_movement) {
    case movement_type::fly:
        walkcost = NP_STANDARDFLYCOST;
        break;

    default:
        walkcost = targetField.getMovementCost();
        break;
    }

    // tile costs are in 1/10s, walk cost in ms
    walkcost *= 100;

    auto agility = std::min(getAttribute(Character::agility), MAX_WALK_AGI);

    if (getType() != player) {
        walkcost += ADDITIONAL_MONSTER_WALKING_COST;
    }

    float agilityModifier = float(10 - agility) / 100.0;
    float loadModifier = relativeLoad() / 10.0 * 3.0;

    walkcost += walkcost * (agilityModifier + loadModifier);

    walkcost = std::min(std::max(walkcost, MIN_WALK_COST), MAX_WALK_COST);

    if (diagonalMove) {
        walkcost *= sqrt2;
    }

    if (running) {
        walkcost *= 0.6;
    }

    return walkcost;
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

bool Character::Warp(const position &newPos) {
    position oldpos = pos;
    position updatedPos = newPos;

    try {
        Field &oldField = _world->fieldAt(pos);
        Field &newField = _world->walkableFieldNear(updatedPos);
        oldField.removeChar();
        setPosition(updatedPos);
        newField.setChar();
        _world->sendCharacterWarpToAllVisiblePlayers(this, oldpos, PUSH);
        return true;
    } catch (FieldNotFound &) {
        return false;
    }
}

bool Character::forceWarp(const position &newPos) {
    position oldpos = pos;

    try {
        Field &oldField = _world->fieldAt(pos);
        Field &newField = _world->fieldAt(newPos);
        oldField.removeChar();
        setPosition(newPos);
        newField.setChar();
        _world->sendCharacterWarpToAllVisiblePlayers(this, oldpos, PUSH);
        return true;
    } catch (FieldNotFound &) {
        return false;
    }
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

void Character::changeQualityAt(unsigned char pos, short int amount) {
    if (pos < MAX_BODY_ITEMS + MAX_BELT_SLOTS) {
        if ((items[ pos ].getId() == 0) || (items[pos].getId() == BLOCKEDITEM)) {
            return;
        }


        short int tmpQuality = ((amount+items[pos].getDurability())<100) ? (amount + items[pos].getQuality()) : (items[pos].getQuality() - items[pos].getDurability() + 99);

        if (tmpQuality%100 > 1) {
            items[ pos ].setQuality(tmpQuality);
            return;
        } else {

            if (pos == RIGHT_TOOL && items[LEFT_TOOL].getId() == BLOCKEDITEM) {
                items[LEFT_TOOL].reset();
            } else if (pos == LEFT_TOOL && items[RIGHT_TOOL].getId() == BLOCKEDITEM) {
                items[RIGHT_TOOL].reset();
            }

            items[ pos ].reset();
            return;
        }
    }
}

void Character::callAttackScript(Character *Attacker, Character *Defender) {
    const auto weaponId = items[RIGHT_TOOL].getId();

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
        if (items[ i ].getId() == id) {
            ScriptItem item(items[ i ]);

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

    if ((items[ BACKPACK ].getId() != 0) && backPackContents) {
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

void Character::logAdmin(const std::string &message) {
}

void Character::updateAppearanceForPlayer(Player *target, bool always) {
    if (!isinvisible) {
        ServerCommandPointer cmd = std::make_shared<AppearanceTC>(this, target);
        target->sendCharAppearance(id, cmd, always);
    }
}

void Character::updateAppearanceForAll(bool always) {
    if (!isinvisible) {
        for (const auto &player : World::get()->Players.findAllCharactersInScreen(pos)) {
            ServerCommandPointer cmd = std::make_shared<AppearanceTC>(this, player);
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
        ServerCommandPointer cmd = std::make_shared<AnimationTC>(id, animID);

        for (const auto &player : World::get()->Players.findAllCharactersInScreen(pos)) {
            player->Connection->addCommand(cmd);
        }
    }
}

bool Character::isNewPlayer() const {
    return false;
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

void Character::setPosition(const position &pos) {
    _world->moveTo(this, pos);
    this->pos = pos;
}

void Character::setRace(TYPE_OF_RACE_ID race) {
    this->race = race;
}

void Character::setFaceTo(face_to faceTo) {
    faceto = faceTo;
}

void Character::setMagicFlags(magic_type type, uint64_t flags) {
    magic.flags[type] = flags;
}

std::ostream &operator<<(std::ostream &os, const Character &character) {
    return os << character.to_string();
};

const MonsterStruct::loottype &Character::getLoot() const {
    throw NoLootFound();
}
