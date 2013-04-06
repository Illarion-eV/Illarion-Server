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


#include "Scheduler.hpp"
#include <iostream>
#include "World.hpp"

//#define Scheduler_DEBUG

Scheduler::Scheduler() : cycle(0), world(World::get()) {
}

void Scheduler::NextCycle() {
    int emexit = 0;

    while (!Tasks.empty() && (emexit < 201) && (Tasks.front()->GetNextCycle() <= cycle)) {
        ++emexit;

        if (emexit>=199) {
            break;
        }

        if (!Tasks.empty()) {
            auto task = std::move(Tasks.front());
            Tasks.pop_front();
        
            if (task) {
                if ((*task)(world)) {
                    AddTask(std::move(task));
                }
            }
        }
    }

    ++cycle;
}

void Scheduler::AddTask(std::unique_ptr<SchedulerObject> task) {
    bool inserted = false;

    if (task->GetNextCycle() <= cycle) {
        task->SetNextCycle(cycle + 1);
    }

    for (auto it = Tasks.begin(); it != Tasks.end(); ++it) {
        if ((*it)->GetNextCycle() > task->GetNextCycle()) {
            Tasks.insert(it, std::move(task));
            inserted = true;
            break;
        }
    }

    if (!inserted) {
        Tasks.push_back(std::move(task));
    }
}

