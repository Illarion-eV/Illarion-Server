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


#ifndef SHEDULER_H
#define SHEDULER_H

#include <list>

class World;

class SchedulerObject {

public:
    //überladung für den Operator ()
    virtual bool operator()(World *world) = 0;
    inline unsigned long int GetNextCycle() {
        return nextCycle;
    }
    inline void SetNextCycle(unsigned long int nCycle) {
        nextCycle = nCycle;
    }
    inline short int GetCount() {
        return count;
    }
    inline void SetCount(unsigned short int nCount) {
        count = nCount;
    }
    inline unsigned long int GetCycleTime() {
        return CycleTime;
    }
    inline void SetCycleTime(unsigned long int nCycleTime) {
        CycleTime = nCycleTime;
    }
    virtual ~SchedulerObject() {}

protected:

    unsigned long int nextCycle ;
    unsigned long int CycleTime ;
    unsigned short int count;

private:
    //! no copy operator for pure virtual classes
    SchedulerObject &operator=(const SchedulerObject &);
};


class Scheduler {

public:
    //constructor welcher den Thread erstellt
    // \param Zeiger auf das World objekt in dem der Scheduler läuft
    Scheduler(World *pworld);
    //Destructor welcher den Thread beendet.
    ~Scheduler();
    //Funktion um einen Task in die Liste hinzu zu fügen
    void AddTask(SchedulerObject *sobject);
    //Funktion welche den nächsten Zyklus einleitet.
    void NextCycle();
    //Funktion welche den Aktuellen Zyklus zurück liefert.
    inline unsigned long int GetCurrentCycle() {
        return cycle;
    }

private:
    //Liste für die Sheduler objekte
    std::list<SchedulerObject *> Tasks;
    //Aktueller Zyklus
    unsigned long int cycle;
    //verbindung zur Spielwelt
    World *world;
};

#endif
