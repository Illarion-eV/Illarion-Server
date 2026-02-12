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

#ifndef QUEST_NODE_TABLE_HPP
#define QUEST_NODE_TABLE_HPP

#include "globals.hpp"
#include "types.hpp"

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>

class LuaScript;

/**
 * @brief Quest script attachment point
 *
 * Represents a script function that should be called when a quest-related
 * event occurs on a specific entity (item, NPC, monster, or trigger).
 */
struct NodeStruct {
    std::string entrypoint;             ///< Function name to call in the script
    std::shared_ptr<LuaScript> script;  ///< Loaded Lua script instance
};

/**
 * @brief Singleton table for quest script attachments to game entities
 *
 * Manages quest-related script bindings loaded from quest.txt files in the
 * questsystem directory. Quest scripts can be attached to:
 * - Items (use item, move item, etc.)
 * - NPCs (talk to NPC, trade with NPC)
 * - Monsters (kill monster, loot monster)
 * - Trigger fields (enter trigger location)
 *
 * Each quest directory contains a quest.txt file defining script attachments:
 * - Format: type, id, function, script (or x, y, z, function, script for triggers)
 * - Multiple scripts can attach to the same entity
 * - Scripts are called in addition to the entity's main scripts
 *
 * This enables modular quest implementation where quest-specific behavior
 * is added without modifying core entity scripts.
 *
 * @note Singleton pattern - access via getInstance()
 * @note Loads from filesystem, not database
 */
class QuestNodeTable {
private:
    template <typename Key> using Table = std::unordered_multimap<Key, NodeStruct>;
    template <typename Key> using TableIterator = typename Table<Key>::const_iterator;

    static std::unique_ptr<QuestNodeTable> instance;
    Table<TYPE_OF_ITEM_ID> itemNodes;     ///< Quest scripts attached to items
    Table<unsigned int> npcNodes;          ///< Quest scripts attached to NPCs
    Table<unsigned int> monsterNodes;      ///< Quest scripts attached to monsters
    Table<position> triggerNodes;          ///< Quest scripts attached to trigger positions

    static constexpr auto normalEntryCount = 4;         ///< Expected fields for item/NPC/monster
    static constexpr auto triggerfieldEntryCount = 6;   ///< Expected fields for triggers

    static constexpr auto typePosition = 0;                   ///< Entry type field index
    static constexpr auto idPosition = 1;                     ///< Entity ID field index
    static constexpr auto functionPosition = 2;               ///< Function name field index
    static constexpr auto scriptPosition = 3;                 ///< Script filename field index
    static constexpr auto triggerCoordinateXPosition = 1;     ///< Trigger X coordinate field index
    static constexpr auto triggerCoordinateYPosition = 2;     ///< Trigger Y coordinate field index
    static constexpr auto triggerCoordinateZPosition = 3;     ///< Trigger Z coordinate field index
    static constexpr auto triggerFunctionPosition = 4;        ///< Trigger function field index
    static constexpr auto triggerScriptPosition = 5;          ///< Trigger script field index

public:
    /**
     * @brief Constructor - loads quest node data from filesystem
     *
     * Scans questsystem directory for quest.txt files and loads all
     * quest script attachments.
     */
    QuestNodeTable();

    QuestNodeTable(const QuestNodeTable &) = delete;
    auto operator=(const QuestNodeTable &) -> QuestNodeTable & = delete;
    QuestNodeTable(QuestNodeTable &&) = delete;
    auto operator=(QuestNodeTable &&) -> QuestNodeTable & = delete;
    ~QuestNodeTable() = default;

    template <typename Key> using TableRange = std::pair<TableIterator<Key>, TableIterator<Key>>;

    /**
     * @brief Get singleton instance
     * @return Reference to the singleton instance
     */
    static auto getInstance() -> QuestNodeTable &;

    /**
     * @brief Reload all quest node data from filesystem
     *
     * Clears existing data and rescans questsystem directory.
     */
    void reload();

    /**
     * @brief Get all quest scripts attached to items
     * @return Iterator range over item quest nodes
     */
    auto getItemNodes() const -> TableRange<TYPE_OF_ITEM_ID>;

    /**
     * @brief Get all quest scripts attached to NPCs
     * @return Iterator range over NPC quest nodes
     */
    auto getNpcNodes() const -> TableRange<unsigned int>;

    /**
     * @brief Get all quest scripts attached to monsters
     * @return Iterator range over monster quest nodes
     */
    auto getMonsterNodes() const -> TableRange<unsigned int>;

    /**
     * @brief Get all quest scripts attached to trigger positions
     * @return Iterator range over trigger quest nodes
     */
    auto getTriggerNodes() const -> TableRange<position>;

private:
    /**
     * @brief Parse a quest.txt file and load its attachments
     * @param questFile Open file stream for quest.txt
     * @param questPath Path to quest directory
     */
    void readQuest(std::ifstream &questFile, std::filesystem::path &questPath);

    /**
     * @brief Clear all quest node data
     */
    void clear();
};

#endif
