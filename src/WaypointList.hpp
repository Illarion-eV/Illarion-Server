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

#ifndef WAYPOINTLIST_HPP
#define WAYPOINTLIST_HPP

#include "globals.hpp"
#include "types.hpp"

#include <list>

class Character;

/**
 * @brief Manages a list of waypoints for character pathfinding and movement.
 *
 * This class maintains a queue of positions that a character should navigate to,
 * and calculates step-by-step movement directions to reach each waypoint in sequence.
 */
class WaypointList {
public:
    static const uint8_t max_fields_for_waypoints = 12; ///< Maximum distance for pathfinding calculations.

    /**
     * @brief Constructs a waypoint list for a specific character.
     * @param movechar The character that will follow these waypoints.
     */
    explicit WaypointList(Character *movechar);

    /**
     * @brief Gets the list of waypoints.
     * @return A const reference to the list of positions.
     */
    [[nodiscard]] auto getWaypoints() const -> const std::list<position> &;

    /**
     * @brief Adds a waypoint to the end of the list.
     * @param pos The position to add as a waypoint.
     */
    void addWaypoint(const position &pos);

    /**
     * @brief Gets the next waypoint without removing it from the list.
     * @param pos Output parameter to receive the next waypoint position.
     * @return true if a waypoint exists, false if the list is empty.
     */
    auto getNextWaypoint(position &pos) const -> bool;

    /**
     * @brief Clears all waypoints from the list.
     */
    void clear();

    /**
     * @brief Executes one movement step toward the next waypoint.
     * @return true if a move was made, false if no more waypoints or movement failed.
     */
    auto makeMove() -> bool;

    /**
     * @brief Recalculates the step list for reaching the next waypoint.
     * @return true if a path was calculated, false if no waypoints remain.
     * @note Automatically removes reached waypoints from the list.
     */
    auto recalcStepList() -> bool;

private:
    std::list<position> positions; ///< Queue of waypoint positions to visit.
    Character *_movechar; ///< The character following this waypoint list.
    std::list<direction> steplist; ///< Calculated step-by-step directions to the next waypoint.

    /**
     * @brief Checks if the character has reached the current waypoint and removes it if so.
     * @return true if waypoints remain, false if the list is empty.
     */
    auto checkPosition() -> bool;
};
#endif
