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

#ifndef DATA_HPP
#define DATA_HPP

#include "data/ArmorObjectTable.hpp"
#include "data/ContainerObjectTable.hpp"
#include "data/ItemTable.hpp"
#include "data/LongTimeEffectTable.hpp"
#include "data/MonsterAttackTable.hpp"
#include "data/NaturalArmorTable.hpp"
#include "data/QuestTable.hpp"
#include "data/RaceTable.hpp"
#include "data/ScriptVariablesTable.hpp"
#include "data/SkillTable.hpp"
#include "data/SpellTable.hpp"
#include "data/TilesModificatorTable.hpp"
#include "data/TilesTable.hpp"
#include "data/TriggerTable.hpp"
#include "data/WeaponObjectTable.hpp"

/**
 * @brief Central data management namespace
 *
 * Provides unified access to all game data tables through singleton instances.
 * This namespace encapsulates:
 * - Table instance management
 * - Coordinated reload operations
 * - Script reloading
 * - Table activation after reload
 *
 * All tables are created as static instances and accessed via getter functions.
 * This ensures single instances and provides a clean API for accessing game data.
 *
 * Typical reload workflow:
 * 1. Call reload() or reloadTables()
 * 2. Data loads into buffers without affecting active data
 * 3. Call reloadScripts() to refresh Lua bindings
 * 4. Call activateTables() to atomically swap in new data
 */
namespace Data {

/**
 * @brief Get script variables table
 * @return Reference to singleton ScriptVariablesTable
 */
auto scriptVariables() -> ScriptVariablesTable &;

/**
 * @brief Get skills table
 * @return Reference to singleton SkillTable
 */
auto skills() -> SkillTable &;

/**
 * @brief Get quests table
 * @return Reference to singleton QuestTable
 */
auto quests() -> QuestTable &;

/**
 * @brief Get races table
 * @return Reference to singleton RaceTable
 */
auto races() -> RaceTable &;

/**
 * @brief Get natural armors table
 * @return Reference to singleton NaturalArmorTable
 */
auto naturalArmors() -> NaturalArmorTable &;

/**
 * @brief Get monster attacks table
 * @return Reference to singleton MonsterAttackTable
 */
auto monsterAttacks() -> MonsterAttackTable &;

/**
 * @brief Get items table
 * @return Reference to singleton ItemTable
 */
auto items() -> ItemTable &;

/**
 * @brief Get weapon items table
 * @return Reference to singleton WeaponObjectTable
 */
auto weaponItems() -> WeaponObjectTable &;

/**
 * @brief Get armor items table
 * @return Reference to singleton ArmorObjectTable
 */
auto armorItems() -> ArmorObjectTable &;

/**
 * @brief Get container items table
 * @return Reference to singleton ContainerObjectTable
 */
auto containerItems() -> ContainerObjectTable &;

/**
 * @brief Get tiles modificator items table
 * @return Reference to singleton TilesModificatorTable
 */
auto tilesModItems() -> TilesModificatorTable &;

/**
 * @brief Get tiles table
 * @return Reference to singleton TilesTable
 */
auto tiles() -> TilesTable &;

/**
 * @brief Get spells table
 * @return Reference to singleton SpellTable
 */
auto spells() -> SpellTable &;

/**
 * @brief Get triggers table
 * @return Reference to singleton TriggerTable
 */
auto triggers() -> TriggerTable &;

/**
 * @brief Get long time effects table
 * @return Reference to singleton LongTimeEffectTable
 */
auto longTimeEffects() -> LongTimeEffectTable &;

/**
 * @brief Get vector of all table pointers
 * @return Vector containing pointers to all tables
 */
auto getTables() -> std::vector<Table *>;

/**
 * @brief Reload all tables from database into buffers
 * @return true if all tables loaded successfully, false if any failed
 */
auto reloadTables() -> bool;

/**
 * @brief Reload all table scripts
 *
 * Refreshes Lua script bindings for all tables that have scripts.
 */
void reloadScripts();

/**
 * @brief Activate buffered data for all tables
 *
 * Atomically swaps buffered data into active storage for all tables,
 * making newly loaded data live.
 */
void activateTables();

/**
 * @brief Complete reload: tables, scripts, and activation
 *
 * Convenience function that performs the full reload sequence:
 * reloadTables() -> reloadScripts() -> activateTables()
 *
 * @return true if reload succeeded, false if any step failed
 */
auto reload() -> bool;

/**
 * @brief Prepare for reload (reserved for future use)
 *
 * Placeholder for pre-reload operations.
 */
void preReload();

/**
 * @brief Find item ID by name
 *
 * Searches the items table for an item with the given name.
 *
 * @param itemName Item name to search for
 * @return Item ID if found, or 0 if not found
 */
auto getIdFromName(const std::string &itemName) -> TYPE_OF_ITEM_ID;

} // namespace Data

#endif
