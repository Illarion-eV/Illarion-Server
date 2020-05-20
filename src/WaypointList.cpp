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


#include "WaypointList.hpp"
#include "World.hpp"
#include "Logger.hpp"

WaypointList::WaypointList(Character *movechar) : _movechar(movechar) {

}

const std::list<position> &WaypointList::getWaypoints() const {
    return positions;
}

void WaypointList::addWaypoint(const position &pos) {
    positions.push_back(pos);
}

bool WaypointList::getNextWaypoint(position &pos) const {
    if (positions.empty()) {
        return false;
    }

    pos = positions.front();
    return true;
}

void WaypointList::clear() {
    positions.clear();
}

bool WaypointList::checkPosition() {
    if (positions.empty()) {
        return false;
    }

    if (_movechar->getPosition() == positions.front()) {
        positions.pop_front();
    }

    return true;
}

bool WaypointList::recalcStepList() {
    if (!checkPosition()) {
        return false;
    }

    if (positions.empty()) {
        return false;
    }

    steplist.clear();
    _movechar->getStepList(positions.front(), steplist);
    return (!steplist.empty());
}

bool WaypointList::makeMove() {
    if (steplist.empty()) {
        if (!recalcStepList()) {
            return false;
        }
    }

    if (!_movechar->move(steplist.front())) {
        return steplist.size() > 1 && recalcStepList();
    }

    steplist.pop_front();
    return true;
}
