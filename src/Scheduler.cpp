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

//#define Scheduler_DEBUG

Scheduler::Scheduler(World *pworld) : cycle(0), world(pworld) {
}

void Scheduler::NextCycle() {
    int emexit = 0;

    while (!Tasks.empty() && (emexit < 201) && (Tasks.front()->GetNextCycle() <= cycle)) {
        SchedulerObject *m_object = nullptr;
        ++emexit;

        if (emexit>=199) {
            break;
        }

        if (!Tasks.empty()) {
            m_object = Tasks.front();
        }

        if (m_object) {
            if ((*m_object)(world)) {
                AddTask(m_object);
            }
        }

        Tasks.pop_front();
    }

    ++cycle;
}

void Scheduler::AddTask(SchedulerObject *sobject) {
    std::list<SchedulerObject *>::iterator it;
    bool inserted = false;

    if (sobject->GetNextCycle() <= cycle) {
        sobject->SetNextCycle(cycle + 1);
    }

    for (it = Tasks.begin(); it != Tasks.end(); ++it) {
        if ((*it)->GetNextCycle() > sobject->GetNextCycle()) {
            Tasks.insert(it, sobject);
            inserted = true;
            break;
        }
    }

    if (!inserted) {
        Tasks.push_back(sobject);
    }
}

