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


#include "Monster.hpp"
#include "Random.hpp"
#include "tuningConstants.hpp"
#include "data/CommonObjectTable.hpp"
#include <iostream>
#include "data/MonsterTable.hpp"
#include <boost/shared_ptr.hpp>
#include "script/LuaMonsterScript.hpp"
#include "World.hpp"
#include "WaypointList.hpp"

std::auto_ptr<IdCounter> Monster::monsteridc;

// the table with monster descriptions
extern MonsterTable *MonsterDescriptions;

Monster::Monster(const TYPE_OF_CHARACTER_ID &type, const position &newpos, SpawnPoint *spawnpoint) throw(unknownIDException)
    : Character(),lastTargetPosition(position(0,0,0)),lastTargetSeen(false), spawn(spawnpoint), monstertype(type) {

    if (monsteridc.get() == 0) {
        monsteridc.reset(new IdCounter(configOptions["monsteridc"], MONSTER_BASE));    // reset monster id to 0xBB000000
    }

    character = monster;
    id = monsteridc->nextFreeId();
    actionPoints = NP_MAX_AP;
    SetAlive(true);
    setType(type);
    pos=newpos;
}

void Monster::performStep(position targetpos) {
    position currentTarget;
    bool hasTarget = waypoints->getNextWaypoint(currentTarget);

    if (!(hasTarget && currentTarget == targetpos)) {
        waypoints->clear();
        waypoints->addWaypoint(targetpos);
        waypoints->recalcStepList();
    }

    if (!waypoints->makeMove()) {
        Character::direction dir = static_cast<Character::direction>(Random::uniform(0, 7));
        move(dir);
        actionPoints -= 20;
    }
}

void Monster::setType(const TYPE_OF_CHARACTER_ID &type) throw(unknownIDException) {
    deleteAllSkills();


    MonsterStruct monsterdef;

    if (! MonsterDescriptions->find(type, monsterdef)) {
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
    race = monsterdef.race;
    _canAttack = monsterdef.canattack;
    name = monsterdef.nameEn;
    nameDe = monsterdef.nameDe;
}

void Monster::setSpawn(SpawnPoint *sp) {
    spawn = sp;
}

Monster::~Monster() {
#ifdef Character_DEBUG
    std::cout << "Monster Destruktor Start" << std::endl;
#endif

    if (spawn) {
        spawn->dead(monstertype);
    }

#ifdef Character_DEBUG
    std::cout << "Monster Destruktor Ende" << std::endl;
#endif
}

void Monster::remove() {
    Character::SetAlive(false);
}

void Monster::SetAlive(bool t) {
    bool wasAlive = Character::IsAlive();
    Character::SetAlive(t);

    if (!t && wasAlive) {

        MonsterStruct monStruct;

        if (MonsterDescriptions->find(getType(), monStruct)) {
            if (monStruct.script) {
                monStruct.script->onDeath(this);
            } else {
                std::cerr<<"Script for Monster: "<<getType()<<" not active!"<<std::endl;
            }
        } else {
            std::cerr<<"Can't finde Description for Monster: " << getType() << " on Death not called!"<<std::endl;
        }
    }
}

bool Monster::attack(Character *target) {

    MonsterStruct monStruct;

    if (MonsterDescriptions->find(getType(), monStruct)) {
        if (monStruct.script) {
            monStruct.script->onAttack(this,target);
        } else {
            std::cerr<<"Script for Monster: "<<getType()<<"not active!"<<std::endl;
        }
    } else {
        std::cerr<<"Can't find Description for Monster: " << getType() << " onAttack not called!" << std::endl;
    }

    return Character::attack(target);
}

void Monster::heal() {
    increaseAttrib("hitpoints", 150);
    increaseAttrib("mana", 150);
}

void Monster::receiveText(talk_type tt, std::string message, Character *cc) {
    MonsterStruct monStruct;

    if (MonsterDescriptions->find(getType(), monStruct)) {
        if (monStruct.script!=NULL && monStruct.script->existsEntrypoint("receiveText")) {
            //Nur Script aufrufen wenn man sich nicht selber h�rt.
            if (this != cc) {
                monStruct.script->receiveText(this,tt,message,cc);
            }
        }
    } else {
        std::cerr<<"Can't find description for monster: " << getType() << " receiveText not called!"<<std::endl;
    }
}
