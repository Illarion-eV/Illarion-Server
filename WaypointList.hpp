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


#ifndef _WAYPOINTLIST_HPP_
#define _WAYPOINTLIST_HPP_


#include "globals.hpp"
#include "luabind/luabind.hpp"
#include "luabind/object.hpp"
#include "Character.hpp"

class WaypointList {
public:
    static const uint8_t max_fields_for_waypoints = 12;
    WaypointList(Character *movechar);
    void addFromList(const luabind::object &list);
    luabind::object getWaypoints();
    void addWaypoint(position pos);
    bool getNextWaypoint(position &pos) const;
    void clear();
    bool makeMove();
    bool recalcStepList();

private:
    std::list<position> positions;
    Character *_movechar;
    std::list<Character::direction> steplist;
    bool checkPosition();
};
#endif

