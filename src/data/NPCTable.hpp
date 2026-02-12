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

#ifndef NPC_TABLE_HPP
#define NPC_TABLE_HPP

#include "NPC.hpp"
#include "World.hpp"
#include "globals.hpp"

#include <list>
#include <string>

class World;

/**
 * @brief Data structure for NPC spawn definition
 *
 * Contains all information needed to spawn an NPC in the world,
 * including position, appearance, behavior, and associated scripts.
 */
struct NPCStruct {
    position NPCPos;                            ///< Spawn position in world
    std::string Name;                           ///< NPC name
    TYPE_OF_RACE_ID type;                       ///< Race/appearance type
    Character::face_to faceto;                  ///< Initial facing direction
    std::vector<struct NPCTalk> speechTexts;    ///< Pre-defined speech texts
    unsigned short int walk_range;              ///< Maximum wander distance from spawn
    bool ishealer;                              ///< Whether NPC can heal players
};

/**
 * @brief Table for NPC spawn definitions and instantiation
 *
 * Loads NPC spawn data from the database "npc" table and creates NPC
 * instances in the world. Each entry defines:
 * - Position and facing direction
 * - Name and race
 * - Appearance details (hair, beard, skin color)
 * - Behavior flags (healer status)
 * - Associated Lua scripts
 *
 * Unlike other tables, NPCTable directly instantiates NPC characters in the
 * world during construction rather than just storing definitions. It also
 * loads quest scripts from QuestNodeTable.
 *
 * @note The reload mechanism needs refactoring (see TODO in implementation)
 *
 * Database table: npc
 */
class NPCTable {
public:
    /**
     * @brief Constructor - loads and spawns all NPCs
     *
     * Queries the npc table, creates NPC character instances, and adds them
     * to the world. Also loads and assigns NPC scripts including quest scripts.
     */
    NPCTable();

    /**
     * @brief Check if NPC data loaded successfully
     * @return true if data is valid, false if loading failed
     */
    [[nodiscard]] auto dataOK() const -> bool { return m_dataOK; };

private:
    /**
     * @brief Load NPC data and spawn NPCs in world
     *
     * Queries database, creates NPC instances, and populates the world.
     */
    void reload();

    bool m_dataOK{};                        ///< Whether data loaded successfully
    World *_world;                          ///< Pointer to game world
    std::list<struct NPCStruct> NPCList;    ///< List of NPC definitions (currently unused)
};

#endif
