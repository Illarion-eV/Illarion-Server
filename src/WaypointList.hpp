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


#ifndef _WAYPOINTLIST_HPP_
#define _WAYPOINTLIST_HPP_


#include "globals.hpp"
#include "types.hpp"
#include <list>

class Character;

class WaypointList {
public:
    static const uint8_t max_fields_for_waypoints = 12;
    explicit WaypointList(Character *movechar);
    [[nodiscard]] auto getWaypoints() const -> const std::list<position> &;
    void addWaypoint(const position &pos);
    auto getNextWaypoint(position &pos) const -> bool;
    void clear();
    auto makeMove() -> bool;
    auto recalcStepList() -> bool;

private:
    std::list<position> positions;
    Character *_movechar;
    std::list<direction> steplist;
    auto checkPosition() -> bool;
};
#endif

