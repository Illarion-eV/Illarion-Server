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

#include "Container.hpp"
#include "Logger.hpp"
#include "LongTimeAction.hpp"
#include "MonitoringClients.hpp"
#include "Player.hpp"
#include "Random.hpp"
#include "World.hpp"
#include "a_star.hpp"
#include "character_ptr.hpp"
#include "constants.hpp"
#include "data/Data.hpp"
#include "data/RaceTypeTable.hpp"
#include "data/TilesTable.hpp"
#include "map/Field.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "script/LuaLearnScript.hpp"
#include "script/LuaWeaponScript.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <range/v3/all.hpp>

#define MAJOR_SKILL_GAP 100

extern std::shared_ptr<LuaLearnScript> learnScript;
extern std::shared_ptr<LuaWeaponScript> standardFightingScript;
extern std::unique_ptr<RaceTypeTable> raceTypes;

Character::attribute_map_t Character::attributeMap = {{"strength", strength},
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
                                                      {"luck", luck}};

Character::attribute_string_map_t Character::attributeStringMap = {{strength, "strength"},
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
                                                                   {luck, "luck"}};

auto Character::getFrontalPosition() const -> position {
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

auto Character::getStepList(const position &goal, std::list<direction> &steps) const -> bool {
    return pathfinding::a_star(pos, goal, steps);
}

auto Character::getNextStepDir(const position &goal, direction &dir) const -> bool {
    std::list<direction> steps;

    getStepList(goal, steps);

    if (!steps.empty()) {
        dir = steps.front();
        return true;
    }
    return false;
}

Character::Character() : Character(appearance()) {}

Character::Character(const appearance &appearance)
        : effects(this), waypoints(this), _world(World::get()), _appearance(appearance), attributes(ATTRIBUTECOUNT) {
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
    magic.flags[MAGE] = 0x00000000;
    magic.flags[PRIEST] = 0x00000000;
    magic.flags[BARD] = 0x00000000;
    magic.flags[DRUID] = 0x00000000;
}

Character::~Character() {
    if (backPackContents != nullptr) {
        delete backPackContents;
        backPackContents = nullptr;
    }

    std::for_each(depotContents.rbegin(), depotContents.rend(),
                  [](const decltype(depotContents)::value_type &value) { delete value.second; });
}

auto Character::getId() const -> TYPE_OF_CHARACTER_ID { return id; }

auto Character::getName() const -> const std::string & { return name; }

auto Character::getActionPoints() const -> short int { return actionPoints; }

auto Character::getMinActionPoints() const -> short int { return NP_MIN_AP; }

auto Character::getMaxActionPoints() const -> short int { return NP_MAX_AP; }

void Character::setActionPoints(short int ap) {
    if (ap > getMaxActionPoints()) {
        actionPoints = getMaxActionPoints();
    } else {
        actionPoints = ap;
    }
}

void Character::increaseActionPoints(short int ap) { setActionPoints(actionPoints + ap); }

auto Character::canAct() const -> bool { return actionPoints >= getMaxActionPoints(); }

auto Character::getFightPoints() const -> short int { return fightPoints; }

auto Character::getMinFightPoints() const -> short int { return NP_MIN_FP; }

auto Character::getMaxFightPoints() const -> short int { return NP_MAX_FP; }

void Character::setFightPoints(short int fp) {
    if (fp > getMaxFightPoints()) {
        fightPoints = getMaxFightPoints();
    } else {
        fightPoints = fp;
    }
}

void Character::increaseFightPoints(short int fp) { setFightPoints(fightPoints + fp); }

auto Character::canFight() const -> bool { return fightPoints >= getMinFightPoints(); }

auto Character::getActiveLanguage() const -> short int { return activeLanguage; }

void Character::setActiveLanguage(short int l) { activeLanguage = l; }

auto Character::getPosition() const -> const position & { return pos; }

auto Character::getAttackMode() const -> bool { return attackmode; }

void Character::setAttackMode(bool attack) { attackmode = attack; }

auto Character::getLastSpokenText() const -> const std::string & { return lastSpokenText; }

auto Character::isInvisible() const -> bool { return isinvisible; }

void Character::setInvisible(bool invisible) { isinvisible = invisible; }

auto toNumber = [](const Item &item) { return item.getNumber(); };

auto Character::countItem(TYPE_OF_ITEM_ID itemid) const -> int {
    using namespace ranges;
    auto hasItemId = [itemid](const Item &item) { return item.getId() == itemid; };
    int count = accumulate(items | view::filter(hasItemId) | view::transform(toNumber), 0);

    if ((items.at(BACKPACK).getId() != 0) && (backPackContents != nullptr)) {
        count += backPackContents->countItem(itemid);
    }

    return count;
}

auto Character::countItemAt(const std::string &where, TYPE_OF_ITEM_ID itemid, script_data_exchangemap const *data) const
        -> int {
    using namespace ranges;
    auto hasItemIdAndData = [itemid, data](const Item &item) {
        return item.getId() == itemid && (data == nullptr || item.hasData(*data));
    };

    if (where == "all") {
        int count = accumulate(items | view::filter(hasItemIdAndData) | view::transform(toNumber), 0);

        if ((items.at(BACKPACK).getId() != 0) && (backPackContents != nullptr)) {
            count += backPackContents->countItem(itemid, data);
        }

        return count;
    }

    if (where == "belt") {
        return accumulate(items | view::slice(MAX_BODY_ITEMS, MAX_BODY_ITEMS + MAX_BELT_SLOTS) |
                                  view::filter(hasItemIdAndData) | view::transform(toNumber),
                          0);
    }

    if (where == "body") {
        return accumulate(
                items | view::take(MAX_BODY_ITEMS) | view::filter(hasItemIdAndData) | view::transform(toNumber), 0);
    }

    if (where == "backpack") {
        int count = 0;
        if ((items.at(BACKPACK).getId() != 0) && (backPackContents != nullptr)) {
            count += backPackContents->countItem(itemid, data);
        }

        return count;
    }

    return 0;
}

auto Character::GetItemAt(unsigned char itempos) -> ScriptItem {
    ScriptItem item;

    if (itempos < MAX_BODY_ITEMS + MAX_BELT_SLOTS) {
        item = static_cast<ScriptItem>(items.at(itempos));
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

auto Character::eraseItem(TYPE_OF_ITEM_ID itemid, int count, script_data_exchangemap const *data) -> int {
    int temp = count;

    if ((items.at(BACKPACK).getId() != 0) && (backPackContents != nullptr)) {
        temp = backPackContents->eraseItem(itemid, temp, data);
    }

    if (temp > 0) {
        // BACKPACK als Item erstmal auslassen
        for (unsigned char i = MAX_BELT_SLOTS + MAX_BODY_ITEMS - 1; i > 0; --i) {
            if ((items.at(i).getId() == itemid && (data == nullptr || items.at(i).hasData(*data))) && (temp > 0)) {
                if (temp >= items.at(i).getNumber()) {
                    temp = temp - items.at(i).getNumber();
                    items.at(i).reset();
                } else {
                    items.at(i).setNumber(items.at(i).getNumber() - temp);
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

auto Character::createAtPos(unsigned char pos, TYPE_OF_ITEM_ID newid, int count) -> int {
    int temp = count;
    Item it;

    if (weightOK(newid, count, nullptr)) {
        const auto &cos = Data::Items[newid];

        if (cos.isValid()) {
            if (!Data::ContainerItems.exists(newid)) {
                if (items.at(pos).getId() == 0) {
                    if (temp > cos.MaxStack) {
                        items.at(pos).setId(newid);
                        items.at(pos).setWear(cos.AgeingSpeed);
                        items.at(pos).setNumber(cos.MaxStack);
                        temp -= cos.MaxStack;
                    } else {
                        items.at(pos).setId(newid);
                        items.at(pos).setWear(cos.AgeingSpeed);
                        items.at(pos).setNumber(temp);
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

auto Character::createItem(Item::id_type id, Item::number_type number, Item::quality_type quality,
                           script_data_exchangemap const *data) -> int {
    int temp = number;
    Item it;

    if (weightOK(id, number, nullptr)) {
        const auto &cos = Data::Items[id];

        if (cos.isValid()) {
            if (Data::ContainerItems.exists(id)) {
                if (items.at(BACKPACK).getId() == 0) {
                    items.at(BACKPACK).setId(id);
                    items.at(BACKPACK).setWear(cos.AgeingSpeed);
                    items.at(BACKPACK).setQuality(quality);
                    items.at(BACKPACK).setData(data);
                    items.at(BACKPACK).setNumber(1);
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
                    if (backPackContents != nullptr) {
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
                        if ((items.at(i).getId() == id) && (items.at(i).equalData(data))) {
                            int itemsToCreate = temp;
                            temp = items.at(i).increaseNumberBy(temp);

                            if (itemsToCreate != temp) {
                                items.at(i).setWear(cos.AgeingSpeed);
                                items.at(i).setQuality(quality);
                            }
                        }
                    }
                }

                if ((temp > 0) && (backPackContents != nullptr)) {
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
                    if (items.at(i).getId() == 0) {
                        items.at(i).setId(id);
                        items.at(i).setWear(cos.AgeingSpeed);
                        items.at(i).setQuality(quality);
                        items.at(i).setData(data);
                        temp = items.at(i).increaseNumberBy(temp);
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

auto Character::increaseAtPos(unsigned char pos, int count) -> int {
    int temp = count;

    if ((pos > 0) && (pos < MAX_BELT_SLOTS + MAX_BODY_ITEMS)) {
        if (weightOK(items.at(pos).getId(), count, nullptr)) {
            temp = items.at(pos).getNumber() + count;
            auto maxStack = items.at(pos).getMaxStack();

            if (temp > maxStack) {
                items.at(pos).setNumber(maxStack);
                temp = temp - maxStack;
            } else if (temp <= 0) {
                bool updateBrightness = World::get()->getItemStatsFromId(items.at(pos).getId()).Brightness > 0;
                temp = count + items.at(pos).getNumber();
                items.at(pos).reset();

                if (updateBrightness) {
                    updateAppearanceForAll(true);
                }
            } else {
                items.at(pos).setNumber(temp);
                temp = 0;
            }
        }
    }

    return temp;
}

auto Character::swapAtPos(unsigned char pos, TYPE_OF_ITEM_ID newid, uint16_t newQuality) -> bool {
    if ((pos > 0) && (pos < MAX_BELT_SLOTS + MAX_BODY_ITEMS)) {
        bool updateBrightness = World::get()->getItemStatsFromId(items.at(pos).getId()).Brightness > 0 ||
                                World::get()->getItemStatsFromId(newid).Brightness > 0;
        items.at(pos).setId(newid);

        if (updateBrightness) {
            updateAppearanceForAll(true);
        }

        if (newQuality > 0) {
            items.at(pos).setQuality(newQuality);
        }

        return true;
    }
    return false;
}

void Character::ageInventory() {
    for (auto &item : items) {
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

    if ((items.at(BACKPACK).getId() != 0) && (backPackContents != nullptr)) {
        backPackContents->doAge(true);
    }

    for (const auto &depot : depotContents) {
        if (depot.second != nullptr) {
            depot.second->doAge(true);
        }
    }
}

void Character::setAlive(bool t) { alive = t; }

auto Character::attack(Character *target) -> bool {
    if ((target != nullptr) && target->isAlive()) {
        if (!actionRunning()) {
            if (target->isAlive()) {
                if (target->getType() == player) {
                    auto *pl = dynamic_cast<Player *>(target);
                    pl->ltAction->actionDisturbed(this);
                }

                callAttackScript(this, target);
            }
        }

        if (getType() == player) {
            if (target->isAlive()) {
                ServerCommandPointer cmd = std::make_shared<BBSendActionTC>(id, 1, "Attacks : " + target->to_string());
                _world->monitoringClientList->sendCommand(cmd);
            } else {
                ServerCommandPointer cmd = std::make_shared<BBSendActionTC>(id, 1, "Killed : " + target->to_string());
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

auto Character::getAttackTarget() const -> character_ptr {
    auto *target = _world->findCharacter(enemyid);
    return character_ptr(target);
}

auto Character::getSkillName(TYPE_OF_SKILL_ID s) const -> std::string {
    if (Data::Skills.exists(s)) {
        return Data::Skills[s].englishName;
    }
    return "unknown skill";
}

auto Character::getSkill(TYPE_OF_SKILL_ID s) const -> unsigned short int {
    auto iterator = skills.find(s);

    if (iterator == skills.end()) {
        return 0;
    }
    return (*iterator).second.major;
}

auto Character::getMinorSkill(TYPE_OF_SKILL_ID s) const -> unsigned short int {
    auto iterator = skills.find(s);

    if (iterator == skills.end()) {
        return 0;
    }
    return (*iterator).second.minor;
}

void Character::setSkinColour(const Colour &c) {
    _appearance.skin = c;
    updateAppearanceForAll(true);
}

auto Character::getSkinColour() const -> Colour { return _appearance.skin; }

void Character::setHairColour(const Colour &c) {
    _appearance.hair = c;
    updateAppearanceForAll(true);
}

auto Character::getHairColour() const -> Colour { return _appearance.hair; }

void Character::setHair(uint8_t hairID) {
    if (raceTypes->isHairAvailable(race, getAttribute(sex), hairID)) {
        _appearance.hairtype = hairID;
        updateAppearanceForAll(true);
    } else {
        Logger::error(LogFacility::Script)
                << "Race " << race << " subtype " << getAttribute(sex) << " has no hair with id " << int(hairID)
                << ". Leaving hair unchanged at " << int(_appearance.hairtype) << "." << Log::end;
    }
}

auto Character::getHair() const -> uint8_t { return _appearance.hairtype; }

void Character::setBeard(uint8_t beardID) {
    if (raceTypes->isBeardAvailable(race, getAttribute(sex), beardID)) {
        _appearance.beardtype = beardID;
        updateAppearanceForAll(true);
    } else {
        Logger::error(LogFacility::Script)
                << "Race " << race << " subtype " << getAttribute(sex) << " has no beard with id " << int(beardID)
                << ". Leaving beard unchanged at " << int(_appearance.beardtype) << "." << Log::end;
    }
}

auto Character::getBeard() const -> uint8_t { return _appearance.beardtype; }

auto Character::setBaseAttribute(Character::attributeIndex attribute, Attribute::attribute_t value) -> bool {
    auto &attrib = attributes[attribute];
    auto oldValue = attrib.getValue();

    if (isBaseAttributeValid(attribute, value)) {
        attrib.setBaseValue(value);
        auto newValue = attrib.getValue();

        if (newValue != oldValue) {
            handleAttributeChange(attribute);
        }

        return true;
    }
    return false;
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

auto Character::getBaseAttribute(Character::attributeIndex attribute) const -> Attribute::attribute_t {
    return attributes[attribute].getBaseValue();
}

auto Character::getAttribute(Character::attributeIndex attribute) const -> Attribute::attribute_t {
    return attributes[attribute].getValue();
}

auto Character::increaseBaseAttribute(Character::attributeIndex attribute, int amount) -> bool {
    auto &attrib = attributes[attribute];
    auto oldValue = attrib.getValue();

    if (isBaseAttributeValid(attribute, attrib.getBaseValue() + amount)) {
        attrib.increaseBaseValue(amount);
        auto newValue = attrib.getValue();

        if (newValue != oldValue) {
            handleAttributeChange(attribute);
        }

        return true;
    }
    return false;
}

auto Character::increaseAttribute(Character::attributeIndex attribute, int amount) -> Attribute::attribute_t {
    auto &attrib = attributes[attribute];
    auto oldValue = attrib.getValue();
    attrib.increaseValue(amount);
    auto newValue = attrib.getValue();

    if (newValue != oldValue) {
        handleAttributeChange(attribute);
    }

    return newValue;
}

auto Character::isBaseAttributeValid(Character::attributeIndex attribute, Attribute::attribute_t value) const -> bool {
    return Data::Races.isBaseAttributeInLimits(getRace(), attribute, value);
}

auto Character::getBaseAttributeSum() const -> uint16_t {
    return attributes[Character::agility].getBaseValue() + attributes[Character::constitution].getBaseValue() +
           attributes[Character::dexterity].getBaseValue() + attributes[Character::essence].getBaseValue() +
           attributes[Character::intelligence].getBaseValue() + attributes[Character::perception].getBaseValue() +
           attributes[Character::strength].getBaseValue() + attributes[Character::willpower].getBaseValue();
}

auto Character::getMaxAttributePoints() const -> uint16_t { return Data::Races.getMaxAttributePoints(getRace()); }

auto Character::saveBaseAttributes() -> bool { return false; }

void Character::handleAttributeChange(Character::attributeIndex attribute) {
    if (attribute == Character::hitpoints) {
        setAlive(getAttribute(hitpoints) > 0);
        _world->sendHealthToAllVisiblePlayers(this, getAttribute(hitpoints));
    }
}

auto Character::isBaseAttribValid(const std::string &name, Attribute::attribute_t value) const -> bool {
    try {
        Character::attributeIndex attribute = attributeMap.at(name);
        return isBaseAttributeValid(attribute, value);
    } catch (...) {
        return false;
    }
}

auto Character::setBaseAttrib(const std::string &name, Attribute::attribute_t value) -> bool {
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

auto Character::getBaseAttrib(const std::string &name) const -> Attribute::attribute_t {
    try {
        Character::attributeIndex attribute = attributeMap.at(name);
        return getBaseAttribute(attribute);
    } catch (...) {
        return 0;
    }
}

auto Character::increaseBaseAttrib(const std::string &name, int amount) -> bool {
    try {
        Character::attributeIndex attribute = attributeMap.at(name);
        return increaseBaseAttribute(attribute, amount);
    } catch (...) {
        return false;
    }
}

auto Character::increaseAttrib(const std::string &name, int amount) -> Attribute::attribute_t {
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

auto Character::setSkill(TYPE_OF_SKILL_ID skill, short int major, short int minor) -> unsigned short int {
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
        }
        iterator->second.major = major;

        iterator->second.minor = minor;

        return iterator->second.major;
    }
}

auto Character::increaseSkill(TYPE_OF_SKILL_ID skill, short int amount) -> unsigned short int {
    if (!Data::Skills.exists(skill)) {
        return 0;
    }

    auto iterator = skills.find(skill);

    if (iterator == skills.end()) {
        skillvalue sv;

        if (amount <= 0) {
            return 0; // Don't add new skill if value <= 0
        }
        if (amount > MAJOR_SKILL_GAP) {
            sv.major = MAJOR_SKILL_GAP;

        } else {
            sv.major = amount;
        }

        skills[skill] = sv;
        return sv.major;
    }
    int temp = iterator->second.major + amount;

    if (temp <= 0) {
        iterator->second.major = 0;

        skills.erase(iterator); // L�chen des Eintrags wenn value <= 0

    } else if (temp > MAJOR_SKILL_GAP) {
        iterator->second.major = MAJOR_SKILL_GAP;

    } else {
        iterator->second.major = temp;
    }

    return (iterator->second.major);
}

auto Character::increaseMinorSkill(TYPE_OF_SKILL_ID skill, short int amount) -> unsigned short int {
    if (!Data::Skills.exists(skill)) {
        return 0;
    }

    auto iterator = skills.find(skill);

    if (iterator == skills.end()) {
        skillvalue sv;

        if (amount <= 0) {
            return 0; // Don't add new skill if value <= 0
        }
        if (amount > 10000) {
            sv.minor = 10000;

        } else {
            sv.minor = amount;
        }

        if (sv.minor >= 10000) {
            sv.minor = 0;
            sv.major++;
        }

        skills[skill] = sv;
        return (sv.major);
    }
    int temp = iterator->second.minor + amount;

    if (temp <= 0) {
        iterator->second.minor = 0;

        iterator->second.major--;

        if (iterator->second.major == 0) {
            skills.erase(iterator); // delete if major == 0
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

auto Character::getSkillValue(TYPE_OF_SKILL_ID s) const -> const skillvalue * {
    auto it = skills.find(s);

    if (it == skills.end()) {
        return nullptr;
    }
    return &(it->second);
}

void Character::learn(TYPE_OF_SKILL_ID skill, uint32_t actionPoints, uint8_t opponent) {
    if (!Data::Skills.exists(skill)) {
        return;
    }

    if (learnScript) {
        learnScript->learn(this, skill, actionPoints, opponent);
    }
}

void Character::deleteAllSkills() { skills.clear(); }

auto Character::isInRange(Character *cc, unsigned short int distancemetric) const -> bool {
    if (cc != nullptr) {
        return isInRangeToField(cc->pos, distancemetric);
    }

    return false;
}

auto Character::isInScreen(const position &pos) const -> bool {
    short int dx = abs(this->pos.x - pos.x);
    short int dy = abs(this->pos.y - pos.y);
    short int dz = abs(this->pos.z - pos.z);

    return dx + dy <= getScreenRange() && -RANGEDOWN <= dz && dz <= RANGEUP;
}

auto Character::getScreenRange() const -> unsigned short int { return 14; }

auto Character::isInRangeToField(const position &m_pos, unsigned short int distancemetric) const -> bool {
    short int dz = abs(m_pos.z - pos.z);
    short int dx = abs(m_pos.x - pos.x);
    short int dy = abs(m_pos.y - pos.y);

    return dx <= distancemetric && dy <= distancemetric && dz == 0;
}

auto Character::distanceMetricToPosition(const position &m_pos) const -> unsigned short int {
    short int dz = abs(pos.z - m_pos.z);
    short int dx = abs(pos.x - m_pos.x);
    short int dy = abs(pos.y - m_pos.y);

    return std::max(dx, std::max(dy, dz));
}

auto Character::distanceMetric(Character *cc) const -> unsigned short int {
    if (cc != nullptr) {
        return distanceMetricToPosition(cc->pos);
    }
    return 0xFFFF;
}

auto Character::maxLoadWeight() const -> unsigned short int { return getAttribute(Character::strength) * 500 + 5000; }

auto Character::LoadWeight() const -> int {
    int load = 0;

    // alle Items bis auf den Rucksack
    for (int i = 1; i < MAX_BODY_ITEMS + MAX_BELT_SLOTS; ++i) {
        load += items.at(i).getWeight();
    }

    // Rucksack
    load += weightContainer(items.at(0).getId(), 1, backPackContents);

    if (load > MAXWEIGHT) {
        return MAXWEIGHT;
    }
    if (load < 0) {
        return 0;
    }
    return load;
}

auto Character::relativeLoad() const -> float { return float(LoadWeight()) / maxLoadWeight(); }

auto Character::loadFactor() const -> LoadLevel {
    auto load = relativeLoad();

    if (load > 1.0) {
        return LoadLevel::overtaxed;
    }
    if (load > 0.75) {
        return LoadLevel::burdened;
    }

    return LoadLevel::unburdened;
}

auto Character::weightOK(TYPE_OF_ITEM_ID id, int count, Container *tcont) const -> bool {
    if (count < 0) {
        return true;
    }

    int realweight = LoadWeight();

    if (tcont != nullptr) {
        return (realweight + weightContainer(id, 1, tcont)) <= maxLoadWeight();
    }
    const auto &itemStruct = Data::Items[id];

    return (realweight + itemStruct.Weight * count) <= maxLoadWeight();
}

auto Character::weightContainer(TYPE_OF_ITEM_ID id, int count, Container *tcont) -> int {
    int temp = 0;

    if (id != 0) {
        const auto &itemStruct = Data::Items[id];

        if (itemStruct.isValid()) {
            if (count > 0) {
                temp = itemStruct.Weight;
            } else {
                temp = 0 - itemStruct.Weight;
            }
        }

        if (tcont != nullptr) {
            try {
                if (count > 0) {
                    temp += tcont->weight();
                } else {
                    temp -= tcont->weight();
                }
            } catch (RecursionException &e) {
                return MAXWEIGHT;
            }
        }
    }

    if (temp > MAXWEIGHT) {
        return MAXWEIGHT;
    }
    return temp;
}

auto Character::GetMovement() const -> movement_type { return _movement; }

void Character::SetMovement(movement_type tmovement) { _movement = tmovement; }

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

auto Character::alterSpokenMessage(const std::string &message, int languageSkill) const -> std::string {
    int counter = 0;
    std::string alteredMessage;

    alteredMessage = message;

    while (message[counter] != 0) {
        if (Random::uniform(0, 70) > languageSkill) {
            alteredMessage[counter] = '*';
        }

        counter++;
    }

    return alteredMessage;
}

auto Character::getLanguageSkill(int languageSkillNumber) -> int { return 100; }

void Character::talk(talk_type tt, const std::string &message) { // only for say, whisper, shout
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

void Character::talk(talk_type tt, const std::string &german,
                     const std::string &english) { // only for say, whisper, shout
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

    if (abs(xoffs) > abs(yoffs)) {
        turn(static_cast<direction>((xoffs > 0) ? 2 : 6));
    } else {
        turn(static_cast<direction>((yoffs > 0) ? 4 : 0));
    }
}

auto Character::move(direction dir, bool active) -> bool {
    World::triggerFieldMove(this, false);

    // if we move we look into that direction...
    if (dir != dir_up && dir != dir_down) {
        faceto = (Character::face_to)dir;
    }

    // check if we can move to our target field
    position newpos = pos;
    newpos.move(dir);

    try {
        map::Field &oldField = _world->fieldAt(pos);
        map::Field &newField = _world->fieldAtOrBelow(newpos);

        if (moveToPossible(newField)) {
            bool diagonalMove = pos.x != newpos.x && pos.y != newpos.y;
            uint16_t movementcost = getMoveTime(newField, diagonalMove, false);
            actionPoints -= movementcost / 100;

            oldField.removeChar();
            newField.setChar();

            setPosition(newpos);

            if (active) {
                _world->sendCharacterMoveToAllVisibleChars(this, movementcost);
            } else {
                _world->sendPassiveMoveToAllVisiblePlayers(this);
            }

            // check if there are teleporters or other special flags on this field
            World::checkFieldAfterMove(this, newField);

            World::triggerFieldMove(this, true);

            return true;
        }
    } catch (FieldNotFound &) {
    }

    return false;
}

auto Character::moveToPossible(const map::Field &field) const -> bool {
    // for monsters/npcs we just use the field infos for now
    return field.moveToPossible();
}

auto Character::getMoveTime(const map::Field &targetField, bool diagonalMove, bool running) const
        -> TYPE_OF_WALKINGCOST {
    static const float sqrt2 = std::sqrt(2.0);
    TYPE_OF_WALKINGCOST walkcost = 0;

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

void Character::introducePlayer(Player * /*unused*/) {
    // overloaded in Player
}

void Character::teachMagic(unsigned char type, unsigned char flag) {
    // overloaded in Player
}

auto Character::Warp(const position &newPos) -> bool {
    position oldpos = pos;

    try {
        map::Field &oldField = _world->fieldAt(pos);
        map::Field &newField = _world->walkableFieldNear(newPos);
        oldField.removeChar();
        setPosition(newField.getPosition());
        newField.setChar();
        _world->sendCharacterWarpToAllVisiblePlayers(this, oldpos, PUSH);
        return true;
    } catch (FieldNotFound &) {
        return false;
    }
}

auto Character::forceWarp(const position &newPos) -> bool {
    position oldpos = pos;

    try {
        map::Field &oldField = _world->fieldAt(pos);
        map::Field &newField = _world->fieldAt(newPos);
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
    // Nothing to do here, overloaded for players
}

void Character::defaultMusic() {
    // Nothing to do here, overloaded for players
}

void Character::inform(const std::string &message, informType type) const {
    // override for char types that need this kind of information
}

void Character::inform(const std::string & /*unused*/, const std::string & /*unused*/, informType type) const {
    // override for char types that need this kind of information
}

void Character::changeQualityAt(unsigned char pos, short int amount) {
    if (pos < MAX_BODY_ITEMS + MAX_BELT_SLOTS) {
        if ((items.at(pos).getId() == 0) || (items.at(pos).getId() == BLOCKEDITEM)) {
            return;
        }

        short int tmpQuality = ((amount + items.at(pos).getDurability()) < 100)
                                       ? (amount + items.at(pos).getQuality())
                                       : (items.at(pos).getQuality() - items.at(pos).getDurability() + 99);

        if (tmpQuality % 100 > 1) {
            items.at(pos).setQuality(tmpQuality);
            return;
        }
        if (pos == RIGHT_TOOL && items.at(LEFT_TOOL).getId() == BLOCKEDITEM) {
            items.at(LEFT_TOOL).reset();

        } else if (pos == LEFT_TOOL && items.at(RIGHT_TOOL).getId() == BLOCKEDITEM) {
            items.at(RIGHT_TOOL).reset();
        }

        items.at(pos).reset();

        return;
    }
}

void Character::callAttackScript(Character *Attacker, Character *Defender) {
    const auto weaponId = items.at(RIGHT_TOOL).getId();

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

auto Character::getQuestProgress(TYPE_OF_QUEST_ID questid, int &time) const -> TYPE_OF_QUESTSTATUS {
    // Nothing to do here, overridden for players
    return 0;
}

auto Character::getItemList(TYPE_OF_ITEM_ID id) const -> std::vector<ScriptItem> {
    std::vector<ScriptItem> list;

    for (unsigned char i = 0; i < MAX_BELT_SLOTS + MAX_BODY_ITEMS; ++i) {
        if (items.at(i).getId() == id) {
            ScriptItem item(items.at(i));

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

    if ((items.at(BACKPACK).getId() != 0) && (backPackContents != nullptr)) {
        backPackContents->addContentToList(id, list);
    }

    return list;
}

auto Character::GetBackPack() const -> Container * { return backPackContents; }

auto Character::GetDepot(uint32_t depotid) const -> Container * {
    auto it = depotContents.find(depotid + 1);

    if (it == depotContents.end()) {
        return nullptr;
    }
    return it->second;
}

auto Character::idleTime() const -> uint32_t {
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

void Character::logAdmin(const std::string &message) {}

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

void Character::forceUpdateAppearanceForAll() { updateAppearanceForAll(true); }

void Character::sendCharDescription(TYPE_OF_CHARACTER_ID id, const std::string &desc) {
    // Nothing to do here, overloaded in Player
}

void Character::performAnimation(uint8_t animID) {
    if (!isinvisible) {
        ServerCommandPointer cmd = std::make_shared<AnimationTC>(id, animID);

        for (const auto &player : World::get()->Players.findAllCharactersInScreen(pos)) {
            player->Connection->addCommand(cmd);
        }
    }
}

auto Character::isNewPlayer() const -> bool { return false; }

auto Character::pageGM(const std::string &ticket) -> bool {
    // Nothing to do here, overloaded in Player
    return false;
}

void Character::setId(TYPE_OF_CHARACTER_ID id) { this->id = id; }

void Character::setName(const std::string &name) { this->name = name; }

void Character::setPosition(const position &pos) {
    _world->moveTo(this, pos);
    this->pos = pos;
}

void Character::setRace(TYPE_OF_RACE_ID race) { this->race = race; }

void Character::setFaceTo(face_to faceTo) { faceto = faceTo; }

void Character::setMagicFlags(magic_type type, uint64_t flags) { magic.flags.at(type) = flags; }

auto operator<<(std::ostream &os, const Character &character) -> std::ostream & { return os << character.to_string(); };

auto Character::getLoot() const -> const MonsterStruct::loottype & { throw NoLootFound(); }
