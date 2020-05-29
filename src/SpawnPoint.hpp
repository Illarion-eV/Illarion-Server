/*
 * Illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of Illarionserver.
 *
 * Illarionserver  is  free  software:  you can redistribute it and/or modify it
 * under the terms of the  GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * Illarionserver is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY;  without  even  the  implied  warranty  of  MERCHANTABILITY  or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Affero General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SPAWNPOINT_HPP
#define SPAWNPOINT_HPP

#include "globals.hpp"
#include <list>

// just declare a class named World...
class World;

//! defines a Spawnpoint
class SpawnPoint {

public:
    //! Creates a new SpawnPoint at <pos>
    explicit SpawnPoint(const position &pos, int Range = 20, uint16_t Spawnrange = 0, uint16_t Min_Spawntime = 1, uint16_t Max_Spawntime = 1, bool Spawnall = false);

    //! Destructor
    ~SpawnPoint();

    void addMonster(TYPE_OF_CHARACTER_ID type, short int count);

    //! load spawnpoints from database
    auto load(const int &id) -> bool;

    void spawn();

    //! callback called by dying monsters belonging to spawnpoint
    void dead(TYPE_OF_CHARACTER_ID type);

    [[nodiscard]] inline auto get_x() const -> int {
        return spawnpos.x;
    }
    [[nodiscard]] inline auto get_y() const -> int {
        return spawnpos.y;
    }
    [[nodiscard]] inline auto get_z() const -> int {
        return spawnpos.z;
    }

    [[nodiscard]] inline auto getRange() const -> int {
        return range;
    }

private:
    // our link to the world...
    World *world;

    position spawnpos;

    //walkrange of the monsters from the spawn
    int range;

    //range of the spawns, in this area the creatures can be spawned
    uint16_t spawnrange;

    //the number of cycles untlin new monsters are spawned
    uint16_t min_spawntime;
    uint16_t max_spawntime;

    //the number of cycles until the next spawn
    uint16_t nextspawntime;

    //should be all monsters respawned in every cycle
    bool spawnall;

    struct SpawnEntryStruct {
        TYPE_OF_CHARACTER_ID typ;
        short int max_count;
        short int akt_count;
    };

    std::list<struct SpawnEntryStruct> SpawnTypes;
};

#endif // NPC_HPP
