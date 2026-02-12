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

#ifndef SCRIPT_VARIABLES_TABLE_HPP
#define SCRIPT_VARIABLES_TABLE_HPP

#include "data/StructTable.hpp"

/**
 * @brief Table for persistent script-accessible variables
 *
 * Loads and manages persistent key-value pairs from the database "scriptvariables"
 * table that can be read and written by Lua scripts. This provides:
 * - Cross-session persistence of script state
 * - Global variables accessible to all scripts
 * - String-based key-value storage
 * - Database-backed persistence
 *
 * Scripts can use this to store:
 * - World state (event progress, timers)
 * - Global counters and flags
 * - Cross-player shared data
 * - Quest progression markers
 *
 * Unlike other tables, this supports write operations (set/remove) and
 * changes are saved back to the database.
 *
 * Database table: scriptvariables
 * Columns: svt_ids (key), svt_string (value)
 */
class ScriptVariablesTable : public StructTable<std::string, std::string> {
public:
    /**
     * @brief Get database table name
     * @return "scriptvariables"
     */
    auto getTableName() const -> std::string override;

    /**
     * @brief Get column names for the scriptvariables table
     * @return Vector containing column names
     */
    auto getColumnNames() -> std::vector<std::string> override;

    /**
     * @brief Extract variable key from database row
     * @param row Database result row
     * @return Variable key from svt_ids column
     */
    auto assignId(const Database::ResultTuple &row) -> std::string override;

    /**
     * @brief Extract variable value from database row
     * @param row Database result row
     * @return Variable value from svt_string column
     */
    auto assignTable(const Database::ResultTuple &row) -> std::string override;

    /**
     * @brief Find a variable by key
     * @param id Variable key to find
     * @param ret Output parameter for the value
     * @return true if variable exists, false otherwise
     */
    auto find(const std::string &id, std::string &ret) -> bool;

    /**
     * @brief Set a string variable value
     * @param id Variable key
     * @param value String value to store
     */
    void set(const std::string &id, const std::string &value);

    /**
     * @brief Set an integer variable value (converted to string)
     * @param id Variable key
     * @param value Integer value to store
     */
    void set(const std::string &id, int32_t value);

    /**
     * @brief Remove a variable
     * @param id Variable key to remove
     * @return true if variable was removed, false if not found
     */
    auto remove(const std::string &id) -> bool;

    /**
     * @brief Save all changes to database
     *
     * Writes modified variables back to the database. Called periodically
     * or on server shutdown to persist changes.
     */
    void save();

    /**
     * @brief Reload variables from database
     * @return true if reload succeeded, false otherwise
     */
    auto reloadBuffer() -> bool override;

    /**
     * @brief Activate buffered data
     *
     * Special handling for first load to avoid clearing modified data.
     */
    void activateBuffer() override;

private:
    using Base = StructTable<std::string, std::string>;
    bool first = true;  ///< Whether this is the first load (skip activation)
};

#endif
