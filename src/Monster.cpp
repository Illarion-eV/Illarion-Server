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

#include "Monster.hpp"

#include "Config.hpp"
#include "Random.hpp"
#include "WaypointList.hpp"
#include "World.hpp"
#include "data/MonsterTable.hpp"
#include "data/RaceTypeTable.hpp"
#include "script/LuaMonsterScript.hpp"
#include "tuningConstants.hpp"

#include <iostream>
#include <memory>

extern std::unique_ptr<MonsterTable> monsterDescriptions;
extern std::unique_ptr<RaceTypeTable> raceTypes;

uint32_t Monster::counter = 0;

Monster::Monster(const TYPE_OF_CHARACTER_ID &type, const position &newpos, SpawnPoint *spawnpoint)
        : lastTargetPosition(position(0, 0, 0)), spawn(spawnpoint), monstertype(type) {
    setId(MONSTER_BASE + counter++ % (NPC_BASE - MONSTER_BASE));
    setMonsterType(type);
    setPosition(newpos);
}

auto Monster::getLoot() const -> const MonsterStruct::loottype & {
    const auto monsterType = getMonsterType();

    if (monsterDescriptions->exists(monsterType)) {
        return (*monsterDescriptions)[monsterType].loot;
    }
    throw NoLootFound();
}

void Monster::performStep(position targetpos) {
    position currentTarget{};
    bool hasTarget = waypoints.getNextWaypoint(currentTarget);

    if (!(hasTarget && currentTarget == targetpos)) {
        waypoints.clear();
        waypoints.addWaypoint(targetpos);
        waypoints.recalcStepList();
    }

    if (!waypoints.makeMove()) {
        auto dir = static_cast<direction>(Random::uniform(minDirection, maxDirection));
        move(dir);
        increaseActionPoints(-NP_WALK_COST);
    }
}

void Monster::setMonsterType(TYPE_OF_CHARACTER_ID type) {
    deleteAllSkills();

    if (!monsterDescriptions->exists(type)) {
        throw unknownIDException();
    }

    const auto &monsterdef = (*monsterDescriptions)[type];

    setAttribute(Character::luck, Random::uniform(monsterdef.attributes.luck.first, monsterdef.attributes.luck.second));
    setAttribute(Character::strength,
                 Random::uniform(monsterdef.attributes.strength.first, monsterdef.attributes.strength.second));
    setAttribute(Character::dexterity,
                 Random::uniform(monsterdef.attributes.dexterity.first, monsterdef.attributes.dexterity.second));
    setAttribute(Character::constitution,
                 Random::uniform(monsterdef.attributes.constitution.first, monsterdef.attributes.constitution.second));
    setAttribute(Character::agility,
                 Random::uniform(monsterdef.attributes.agility.first, monsterdef.attributes.agility.second));
    setAttribute(Character::intelligence,
                 Random::uniform(monsterdef.attributes.intelligence.first, monsterdef.attributes.intelligence.second));
    setAttribute(Character::perception,
                 Random::uniform(monsterdef.attributes.perception.first, monsterdef.attributes.perception.second));
    setAttribute(Character::willpower,
                 Random::uniform(monsterdef.attributes.willpower.first, monsterdef.attributes.willpower.second));
    setAttribute(Character::essence,
                 Random::uniform(monsterdef.attributes.essence.first, monsterdef.attributes.essence.second));
    setAttribute(Character::hitpoints, monsterdef.hitpoints);
    setAttribute(Character::height, Random::uniform(monsterdef.minsize, monsterdef.maxsize));

    // set skills
    for (const auto &skill : monsterdef.skills) {
        increaseSkill(skill.first, Random::uniform(skill.second.first, skill.second.second));
    }

    // add items
    for (const auto &item : monsterdef.items) {
        auto inventorySlot = item.first;
        auto possibleItems = item.second;
        int numberOfPossibleItems = possibleItems.size();

        if (numberOfPossibleItems > 0) {
            const int selectedItemIndex = [&] {
                if (numberOfPossibleItems == 1) {
                    return 0;
                }

                return Random::uniform(0, numberOfPossibleItems - 1);
            }();

            auto &selectedItem = possibleItems[selectedItemIndex];

            items.at(inventorySlot).setId(selectedItem.itemid);
            items.at(inventorySlot).setNumber(Random::uniform(selectedItem.amount.first, selectedItem.amount.second));
            items.at(inventorySlot).setWear(selectedItem.AgeingSpeed);
        }
    }

    SetMovement(monsterdef.movement);
    setRace(monsterdef.race);
    _canAttack = monsterdef.canattack;
    setName(monsterdef.nameEn);
    nameDe = monsterdef.nameDe;

    const auto raceConfiguration = raceTypes->getRandomRaceConfiguration(monsterdef.race);
    setAttribute(sex, raceConfiguration.subType);
    setHair(raceConfiguration.hair);
    setBeard(raceConfiguration.beard);
    setHairColour(raceConfiguration.hairColour);
    setSkinColour(raceConfiguration.skinColour);
    setSkinColour(raceConfiguration.skinColour);
}

void Monster::setSpawn(SpawnPoint *sp) { spawn = sp; }

Monster::~Monster() {
    if (spawn != nullptr) {
        spawn->dead(monstertype);
    }
}

void Monster::remove() { Character::setAlive(false); }

void Monster::setAlive(bool t) {
    bool wasAlive = Character::isAlive();
    Character::setAlive(t);

    if (!t && wasAlive) {
        if (monsterDescriptions->exists(getMonsterType())) {
            const auto &monStruct = (*monsterDescriptions)[getMonsterType()];

            if (monStruct.script) {
                monStruct.script->onDeath(this);
            }
        }
    }
}

auto Monster::attack(Character *target) -> bool {
    if (monsterDescriptions->exists(getMonsterType())) {
        const auto &monStruct = (*monsterDescriptions)[getMonsterType()];

        if (monStruct.script) {
            monStruct.script->onAttack(this, target);
        }
    }

    return Character::attack(target);
}

void Monster::heal() {
    increaseAttrib("hitpoints", monsterSelfHealAmount);
    increaseAttrib("mana", monsterSelfHealAmount);
}

void Monster::receiveText(talk_type tt, const std::string &message, Character *cc) {
    if (monsterDescriptions->exists(getMonsterType())) {
        const auto &monStruct = (*monsterDescriptions)[getMonsterType()];

        if (monStruct.script && monStruct.script->existsEntrypoint("receiveText")) {
            if (this != cc) {
                monStruct.script->receiveText(this, tt, message, cc);
            }
        }
    }
}

auto Monster::to_string() const -> std::string {
    return "Monster of race " + std::to_string((int)getRace()) + "(" + std::to_string(getId()) + ")";
}
