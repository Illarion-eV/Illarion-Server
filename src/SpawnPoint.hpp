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

/**
 * @brief Manages automatic spawning of monsters at a specific location.
 *
 * A SpawnPoint tracks multiple monster types and their spawn counts, automatically
 * creating new monsters at regular intervals when they die or when spawn time elapses.
 * Monsters can spawn within a defined range and have movement restrictions.
 */
class SpawnPoint {
public:
    /**
     * @brief Creates a new SpawnPoint at the specified position.
     * @param pos The center position for this spawn point.
     * @param Range Maximum walking distance from spawn position for spawned monsters.
     * @param Spawnrange Radius around spawn position where monsters can initially appear.
     * @param Min_Spawntime Minimum cycles before attempting a new spawn.
     * @param Max_Spawntime Maximum cycles before attempting a new spawn.
     * @param Spawnall If true, spawn all missing monsters each cycle; if false, spawn random subset.
     */
    explicit SpawnPoint(const position &pos, Coordinate Range = defaultWalkRange, Coordinate Spawnrange = 0,
                        uint16_t Min_Spawntime = 1, uint16_t Max_Spawntime = 1, bool Spawnall = false);

    /**
     * @brief Adds a monster type to this spawn point.
     * @param type The monster type ID to spawn.
     * @param count The maximum number of this monster type to maintain.
     * @note If the type already exists, the count is added to the existing maximum.
     */
    void addMonster(TYPE_OF_CHARACTER_ID type, int count);

    /**
     * @brief Loads spawn point configuration from the database.
     * @param id The database ID of the spawn point to load.
     * @return true if loaded successfully, false otherwise.
     */
    auto load(const int &id) -> bool;

    /**
     * @brief Attempts to spawn monsters if spawn time has elapsed.
     * @note Spawning only occurs if world spawning is enabled and spawn timer reaches zero.
     */
    void spawn();

    /**
     * @brief Callback invoked when a monster belonging to this spawn point dies.
     * @param type The monster type ID that died.
     * @note Decrements the active count for that monster type.
     */
    void dead(TYPE_OF_CHARACTER_ID type);

    /**
     * @brief Gets the X coordinate of the spawn position.
     * @return The X coordinate.
     */
    [[nodiscard]] inline auto get_x() const -> Coordinate { return spawnpos.x; }

    /**
     * @brief Gets the Y coordinate of the spawn position.
     * @return The Y coordinate.
     */
    [[nodiscard]] inline auto get_y() const -> Coordinate { return spawnpos.y; }

    /**
     * @brief Gets the Z coordinate of the spawn position.
     * @return The Z coordinate.
     */
    [[nodiscard]] inline auto get_z() const -> Coordinate { return spawnpos.z; }

    /**
     * @brief Gets the maximum walking range for spawned monsters.
     * @return The walking range in map coordinates.
     */
    [[nodiscard]] inline auto getRange() const -> Coordinate { return range; }

private:
    World *world; ///< Link to the game world.

    position spawnpos; ///< Center position of the spawn point.

    Coordinate range; ///< Maximum walking distance from spawn for spawned monsters.

    Coordinate spawnrange; ///< Radius around spawn position where monsters can initially appear.

    uint16_t min_spawntime; ///< Minimum cycles before spawning.
    uint16_t max_spawntime; ///< Maximum cycles before spawning.

    uint16_t nextspawntime; ///< Cycles remaining until next spawn attempt.

    bool spawnall; ///< If true, spawn all missing monsters; if false, spawn random subset.

    /**
     * @brief Tracks spawn information for a single monster type.
     */
    struct SpawnEntryStruct {
        TYPE_OF_CHARACTER_ID typ; ///< Monster type ID.
        int max_count; ///< Maximum number of this monster type to maintain.
        int akt_count; ///< Current number of active monsters of this type.
    };

    std::list<struct SpawnEntryStruct> SpawnTypes; ///< List of monster types managed by this spawn point.

    static constexpr Coordinate defaultWalkRange = 20; ///< Default walking range for monsters.
};

#endif
