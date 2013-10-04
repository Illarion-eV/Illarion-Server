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


#ifndef SCHEDULERTASKCLASSES
#define SCHEDULEDTASKCLASSES

#include "Scheduler.hpp"
#include "Player.hpp"
#include "Monster.hpp"
#include "NPC.hpp"

#include "script/LuaLearnScript.hpp"

extern std::shared_ptr<LuaLearnScript>learnScript;

namespace {
    auto reduceMC = [](Character * character) {
        if (character->getMentalCapacity() > 0) {
            learnScript->reduceMC(character);
        }
    };
}

class SGlobalPlayerLearnrate : public SchedulerObject {

public:

    SGlobalPlayerLearnrate(unsigned long int first_cycle) {
        nextCycle = first_cycle;
    }
    virtual ~SGlobalPlayerLearnrate() {}

    bool operator()(World *world) {
        world->Players.for_each(reduceMC);
        nextCycle += 10;
        return true;
    }

};

class SGlobalMonsterLearnrate : public SchedulerObject {

public:

    SGlobalMonsterLearnrate(unsigned long int first_cycle) {
        nextCycle = first_cycle;
    }
    virtual ~SGlobalMonsterLearnrate() {}
    bool operator()(World *world) {
        world->Monsters.for_each(reduceMC);
        world->Npc.for_each(reduceMC);
        nextCycle += 10;
        return true;
    }
};

#endif

