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
#ifndef TRIGGER_TABLE_HPP
#define TRIGGER_TABLE_HPP

#include "data/QuestScriptStructTable.hpp"
#include "globals.hpp"
#include "script/LuaTriggerScript.hpp"

/**
 * @brief Data structure for trigger field properties
 *
 * Stores the position and associated script for a trigger field. Trigger
 * fields are world tiles that execute scripts when characters enter them.
 */
struct TriggerStruct {
    position pos{};                                ///< World position of trigger
    std::string scriptname;                        ///< Name of trigger script file
    std::shared_ptr<LuaTriggerScript> script;      ///< Loaded script instance
};

/**
 * @brief Table for trigger field definitions and scripts
 *
 * Loads trigger field data from the database "triggerfields" table, mapping
 * world positions to trigger scripts. Trigger fields are invisible tiles that
 * execute Lua scripts when characters step on them, enabling:
 * - Area transitions (teleports, zone changes)
 * - Environmental effects (damage, buffs)
 * - Quest progression triggers
 * - Trap activation
 *
 * Uses position as the unique identifier since each tile can only have one
 * trigger. Supports both main trigger scripts and quest-specific trigger
 * scripts via QuestScriptStructTable.
 *
 * @note ScriptParameter is position (not TriggerStruct), so scripts are
 *       constructed with position only
 *
 * Database table: triggerfields
 * Columns: tgf_posx, tgf_posy, tgf_posz, tgf_script
 * Script type: LuaTriggerScript
 */
class TriggerTable : public QuestScriptStructTable<position, TriggerStruct, LuaTriggerScript, position> {
public:
    /**
     * @brief Get database table name
     * @return "triggerfields"
     */
    auto getTableName() const -> std::string override;

    /**
     * @brief Get column names for the triggerfields table
     * @return Vector containing trigger table column names
     */
    auto getColumnNames() -> std::vector<std::string> override;

    /**
     * @brief Extract position from database row
     * @param row Database result row
     * @return Position struct from tgf_posx/y/z columns
     */
    auto assignId(const Database::ResultTuple &row) -> position override;

    /**
     * @brief Parse database row into TriggerStruct
     * @param row Database result row
     * @return TriggerStruct with position populated
     */
    auto assignTable(const Database::ResultTuple &row) -> TriggerStruct override;

    /**
     * @brief Extract script filename from database row
     * @param row Database result row
     * @return Script filename from tgf_script column
     */
    auto assignScriptName(const Database::ResultTuple &row) -> std::string override;

    /**
     * @brief Get quest script nodes for triggers
     * @return Iterator range over trigger quest nodes
     */
    auto getQuestScripts() -> NodeRange override;
};

#endif
