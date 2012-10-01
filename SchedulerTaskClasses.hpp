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


#ifndef SCHEDULERTASKCLASSES
#define SCHEDULEDTASKCLASSES

#include "Scheduler.hpp"
#include "types.hpp"
#include "TableStructs.hpp"
#include "data/CommonObjectTable.hpp"
#include "script/LuaLearnScript.hpp"

extern CommonObjectTable *CommonItems;
extern boost::shared_ptr<LuaLearnScript>learnScript;

class SGlobalPlayerLearnrate : public SchedulerObject {

public:

    SGlobalPlayerLearnrate(unsigned long int first_cycle) {
        nextCycle = first_cycle;
    }
    virtual ~SGlobalPlayerLearnrate() {}

    bool operator()(World *world) {
        ccharactervector < Player * >::iterator playerIterator;

        for (playerIterator = world->Players.begin(); playerIterator < world->Players.end(); ++playerIterator) {
            if ((*playerIterator)->getMentalCapacity() > 0) {   //Prüfen ob aktueller Spieler MC > 0 hat
                learnScript->reduceMC(*playerIterator);
            }
        }

        nextCycle += 10; //Neue Zykluszeit berechnen (aller 10 Zyklen);
        return true; //Task erneut einfügen
    }

};

class SGlobalMonsterLearnrate : public SchedulerObject {

public:

    SGlobalMonsterLearnrate(unsigned long int first_cycle) {
        nextCycle = first_cycle;
    }
    virtual ~SGlobalMonsterLearnrate() {}
    bool operator()(World *world) {
        ccharactervector < Monster * >::iterator monsterIterator;

        for (monsterIterator = world->Monsters.begin(); monsterIterator < world->Monsters.end(); ++monsterIterator) {
            if ((*monsterIterator)->getMentalCapacity() > 0) {   //Prüfen ob aktuelles Monster MC > 0 hat
                learnScript->reduceMC(*monsterIterator);
            }
        }

        ccharactervector < NPC * >::iterator npcIterator;

        for (npcIterator = world->Npc.begin(); npcIterator < world->Npc.end(); ++npcIterator) {
            if ((*npcIterator)->getMentalCapacity() > 0) {
                learnScript->reduceMC(*npcIterator);
            }
        }

        nextCycle += 10; //Neue Zykluszeit berechnen (aller 30 Zyklen) zur Serverentlastung dafür die dreifache Senkung;
        return true; //Task erneut einfügen
    }
};

#endif

