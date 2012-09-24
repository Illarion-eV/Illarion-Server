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

std::auto_ptr<IdCounter> Monster::monsteridc;

// the table with monster descriptions
extern MonsterTable *MonsterDescriptions;

Monster::Monster(const TYPE_OF_CHARACTER_ID &type, const position &newpos, SpawnPoint *spawnpoint) throw(unknownIDException)
    : Character(),lastTargetPosition(position(0,0,0)),lastTargetSeen(false), spawn(spawnpoint), monstertype(type) {

    if (monsteridc.get() == 0) {
        monsteridc.reset(new IdCounter(configOptions["monsteridc"], MONSTER_BASE));    // reset monster id to 0xBB000000
    }

    character = monster;
    actionPoints = NP_MAX_AP;
    SetAlive(true);
    setType(type);
    pos=newpos;
}

void Monster::performStep(position targetpos) {
    Character::direction dir;

    if (getNextStepDir(targetpos, MAX_PATH_FIND, dir)) {
        std::cout << "performStep here!   " << dir << std::endl;
        move(dir);
        std::cout << "performStep here! pass 1" << std::endl;
    } else {
        dir = static_cast<Character::direction>(unsignedShortRandom(0, 7));
        move(dir);
    }
}

void Monster::setType(const TYPE_OF_CHARACTER_ID &type) throw(unknownIDException) {
    deleteAllSkills();


    MonsterStruct monsterdef;

    if (! MonsterDescriptions->find(type, monsterdef)) {
        throw unknownIDException();
    }

    // set attributes
    battrib.luck = battrib.trueluck = rnd(monsterdef.attributes.luck);
    battrib.strength = battrib.truestrength = rnd(monsterdef.attributes.strength);
    battrib.dexterity = battrib.truedexterity = rnd(monsterdef.attributes.dexterity);
    battrib.constitution = battrib.trueconstitution = rnd(monsterdef.attributes.constitution);
    battrib.agility = battrib.trueagility = rnd(monsterdef.attributes.agility);
    battrib.intelligence = battrib.trueintelligence = rnd(monsterdef.attributes.intelligence);
    battrib.perception = battrib.trueperception = rnd(monsterdef.attributes.perception);
    battrib.willpower = battrib.truewillpower = rnd(monsterdef.attributes.willpower);
    battrib.essence = battrib.trueessence = rnd(monsterdef.attributes.essence);
    battrib.hitpoints = battrib.truehitpoints = monsterdef.hitpoints; //Setzen der Hitpoints
    battrib.body_height = battrib.truebody_height = rand()%(monsterdef.maxsize-monsterdef.minsize+1) + monsterdef.minsize;

    // set skills
    for (auto it = monsterdef.skills.begin(); it != monsterdef.skills.end(); ++it) {
        increaseSkill(5, it->first, rnd(it->second));
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
                selectedItemIndex = rnd(0, numberOfPossibleItems - 1);
            }

            auto &selectedItem = possibleItems[selectedItemIndex];

            characterItems[inventorySlot].setId(selectedItem.itemid);
            characterItems[inventorySlot].setNumber(rnd(selectedItem.amount));
            characterItems[inventorySlot].setWear(selectedItem.AgeingSpeed);
        }
    }

    SetMovement(monsterdef.movement);
    race = monsterdef.race;
    _canAttack = monsterdef.canattack;
    id=monsteridc->nextFreeId();
    char mname[ 80 ];
    sprintf(mname, "%0#6X %s", id, monsterdef.name.c_str());
    name=mname;
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
    battrib.truehitpoints = 0;
    battrib.hitpoints = battrib.truehitpoints;
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

bool Monster::attack(Character *target, int &sound, bool &updateInv) {

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

    return Character::attack(target,sound,updateInv);
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
