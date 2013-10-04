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


#ifndef _SCHEDULER_HPP_
#define _SCHEDULER_HPP_

#include <memory>
#include <list>

class World;

class SchedulerObject {

public:
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
    SchedulerObject &operator=(const SchedulerObject &);
};


class Scheduler {

public:
    Scheduler();
    void AddTask(std::unique_ptr<SchedulerObject> sobject);
    void NextCycle();
    inline unsigned long int GetCurrentCycle() {
        return cycle;
    }

private:
    std::list<std::unique_ptr<SchedulerObject>> Tasks;
    unsigned long int cycle;
    World *world;
};

#endif
