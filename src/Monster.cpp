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
#include "Random.hpp"
#include "tuningConstants.hpp"
#include <iostream>
#include "script/LuaMonsterScript.hpp"
#include "World.hpp"
#include "WaypointList.hpp"
#include "Config.hpp"

uint32_t Monster::counter = 0;

Monster::Monster(const TYPE_OF_CHARACTER_ID &type, const position &newpos, SpawnPoint *spawnpoint) throw(unknownIDException)
    : Character(),lastTargetPosition(position(0,0,0)),lastTargetSeen(false), spawn(spawnpoint), monstertype(type) {
    setId(MONSTER_BASE + counter++ % (NPC_BASE-MONSTER_BASE));
    setAlive(true);
    setMonsterType(type);
    setPosition(newpos);
}

void Monster::performStep(position targetpos) {
    position currentTarget;
    bool hasTarget = waypoints.getNextWaypoint(currentTarget);

    if (!(hasTarget && currentTarget == targetpos)) {
        waypoints.clear();
        waypoints.addWaypoint(targetpos);
        waypoints.recalcStepList();
    }

    if (!waypoints.makeMove()) {
        direction dir = static_cast<direction>(Random::uniform(0, 7));
        move(dir);
        increaseActionPoints(-20);
    }
}

void Monster::setMonsterType(const TYPE_OF_CHARACTER_ID &type) throw(unknownIDException) {
    deleteAllSkills();


    MonsterStruct monsterdef;

    if (! World::get()->getMonsterDefinition(type, monsterdef)) {
        throw unknownIDException();
    }

    // set attributes
    setAttribute(Character::luck, Random::uniform(monsterdef.attributes.luck.first, monsterdef.attributes.luck.second));
    setAttribute(Character::strength, Random::uniform(monsterdef.attributes.strength.first, monsterdef.attributes.strength.second));
    setAttribute(Character::dexterity, Random::uniform(monsterdef.attributes.dexterity.first, monsterdef.attributes.dexterity.second));
    setAttribute(Character::constitution, Random::uniform(monsterdef.attributes.constitution.first, monsterdef.attributes.constitution.second));
    setAttribute(Character::agility, Random::uniform(monsterdef.attributes.agility.first, monsterdef.attributes.agility.second));
    setAttribute(Character::intelligence, Random::uniform(monsterdef.attributes.intelligence.first, monsterdef.attributes.intelligence.second));
    setAttribute(Character::perception, Random::uniform(monsterdef.attributes.perception.first, monsterdef.attributes.perception.second));
    setAttribute(Character::willpower, Random::uniform(monsterdef.attributes.willpower.first, monsterdef.attributes.willpower.second));
    setAttribute(Character::essence, Random::uniform(monsterdef.attributes.essence.first, monsterdef.attributes.essence.second));
    setAttribute(Character::hitpoints, monsterdef.hitpoints);
    setAttribute(Character::height, Random::uniform(monsterdef.minsize, monsterdef.maxsize));

    // set skills
    for (auto it = monsterdef.skills.begin(); it != monsterdef.skills.end(); ++it) {
        increaseSkill(it->first, Random::uniform(it->second.first, it->second.second));
    }

    // add items
    for (auto it = monsterdef.items.begin(); it != monsterdef.items.end(); ++it) {

        auto inventorySlot = it->first;
        auto possibleItems = it->second;
        int numberOfPossibleItems = possibleItems.size();

        if (numberOfPossibleItems > 0) {

            int selectedItemIndex;

            if (numberOfPossibleItems == 1) {
                selectedItemIndex = 0;
            } else {
                selectedItemIndex = Random::uniform(0, numberOfPossibleItems - 1);
            }

            auto &selectedItem = possibleItems[selectedItemIndex];

            characterItems[inventorySlot].setId(selectedItem.itemid);
            characterItems[inventorySlot].setNumber(Random::uniform(selectedItem.amount.first, selectedItem.amount.second));
            characterItems[inventorySlot].setWear(selectedItem.AgeingSpeed);
        }
    }

    SetMovement(monsterdef.movement);
    setRace(monsterdef.race);
    _canAttack = monsterdef.canattack;
    setName(monsterdef.nameEn);
    nameDe = monsterdef.nameDe;
}

void Monster::setSpawn(SpawnPoint *sp) {
    spawn = sp;
}

Monster::~Monster() {
    if (spawn) {
        spawn->dead(monstertype);
    }
}

void Monster::remove() {
    Character::setAlive(false);
}

void Monster::setAlive(bool t) {
    bool wasAlive = Character::isAlive();
    Character::setAlive(t);

    if (!t && wasAlive) {

        MonsterStruct monStruct;

        if (World::get()->getMonsterDefinition(getMonsterType(), monStruct)) {
            if (monStruct.script) {
                monStruct.script->onDeath(this);
            }
        }
    }
}

bool Monster::attack(Character *target) {

    MonsterStruct monStruct;

    if (World::get()->getMonsterDefinition(getMonsterType(), monStruct)) {
        if (monStruct.script) {
            monStruct.script->onAttack(this,target);
        }
    }

    return Character::attack(target);
}

void Monster::heal() {
    increaseAttrib("hitpoints", 150);
    increaseAttrib("mana", 150);
}

void Monster::receiveText(talk_type tt, const std::string &message, Character *cc) {
    MonsterStruct monStruct;

    if (World::get()->getMonsterDefinition(getMonsterType(), monStruct)) {
        if (monStruct.script && monStruct.script->existsEntrypoint("receiveText")) {
            if (this != cc) {
                monStruct.script->receiveText(this, tt, message, cc);
            }
        }
    }
}

std::string Monster::to_string() const {
    return "Monster of race " + std::to_string((int)getRace()) + "(" + std::to_string(getId()) + ")";
}
