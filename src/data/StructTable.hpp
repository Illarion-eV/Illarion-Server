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

#ifndef STRUCT_TABLE_HPP
#define STRUCT_TABLE_HPP

#include "Logger.hpp"
#include "data/Table.hpp"
#include "db/Result.hpp"
#include "db/SelectQuery.hpp"

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief Template base class for tables storing structured data indexed by ID
 *
 * Provides a generic implementation of the Table interface for tables that
 * store structured data (StructType) indexed by an ID (IdType). Data is
 * loaded from database tables and stored in an unordered_map for fast lookup.
 *
 * This class implements double-buffering for safe runtime reloading:
 * - Data is loaded into structBuffer via reloadBuffer()
 * - Active data remains in structs during loading
 * - activateBuffer() atomically swaps the two containers
 *
 * Derived classes must implement:
 * - getTableName(): Return database table name
 * - getColumnNames(): Return list of columns to query
 * - assignId(): Extract ID from a database row
 * - assignTable(): Parse database row into StructType
 *
 * @tparam IdType Type of the unique identifier (e.g., TYPE_OF_ITEM_ID, TYPE_OF_SKILL_ID)
 * @tparam StructType Type of the data structure stored (e.g., CommonStruct, SkillStruct)
 */
template <typename IdType, typename StructType> class StructTable : public Table {
    using ContainerType = std::unordered_map<IdType, StructType>;

public:
    /**
     * @brief Load table data from database into buffer
     *
     * Queries the database table and loads all rows into the buffer container.
     * Uses the derived class implementations of getTableName(), getColumnNames(),
     * assignId(), and assignTable() to construct the query and parse results.
     *
     * @return true if loading succeeded, false if an exception occurred
     */
    auto reloadBuffer() -> bool override {
        try {
            Database::SelectQuery query;

            for (const auto &column : getColumnNames()) {
                query.addColumn(column);
            }

            query.setServerTable(getTableName());
            const auto results = query.execute();

            clear();

            for (const auto &row : results) {
                evaluateRow(row);
            }

            isBufferValid = true;
        } catch (std::exception &e) {
            Logger::warn(LogFacility::Database)
                    << "Exception in loading table " << getTableName() << ": " << e.what() << Log::end;
            isBufferValid = false;
        }

        return isBufferValid;
    }

    /**
     * @brief Reload associated scripts (no-op for basic StructTable)
     *
     * Override in derived classes that have script bindings to reload.
     */
    void reloadScripts() override {}

    /**
     * @brief Swap buffer data into active storage
     *
     * Atomically swaps the buffered data container with the active one,
     * then clears the buffer for the next reload cycle.
     */
    void activateBuffer() override {
        structs.swap(structBuffer);
        isBufferValid = false;
        clear();
    }

    /**
     * @brief Check if an entry with the given ID exists
     * @param id The ID to check
     * @return true if entry exists, false otherwise
     */
    auto exists(const IdType &id) const -> bool { return structs.count(id) > 0; }

    /**
     * @brief Access entry by ID with bounds checking
     *
     * Returns the entry for the given ID. If the ID doesn't exist, logs an
     * error and creates a default entry.
     *
     * @param id The ID to look up
     * @return Reference to the entry
     * @note Creates a default entry if ID not found
     */
    auto operator[](const IdType &id) -> const StructType & {
        try {
            return structs.at(id);
        } catch (std::out_of_range &) {
            Logger::error(LogFacility::Script)
                    << "Table " << getTableName() << ": entry " << id << " was not found!" << Log::end;
            return structs[id];
        }
    }

    /**
     * @brief Get entry by ID (const version)
     * @param id The ID to look up
     * @return Const reference to the entry
     * @throws std::out_of_range if ID not found
     */
    auto get(const IdType &id) const -> const StructType & { return structs.at(id); }

    /**
     * @brief Get iterator to beginning of table
     * @return Const iterator to first entry
     */
    auto begin() const -> typename ContainerType::const_iterator { return structs.cbegin(); }

    /**
     * @brief Get iterator to end of table
     * @return Const iterator to one past last entry
     */
    auto end() const -> typename ContainerType::const_iterator { return structs.cend(); }

protected:
    /**
     * @brief Get database table name
     * @return Name of the database table to query
     */
    [[nodiscard]] virtual auto getTableName() const -> std::string = 0;

    /**
     * @brief Get list of column names to query
     * @return Vector of column names for the SELECT query
     */
    virtual auto getColumnNames() -> std::vector<std::string> = 0;

    /**
     * @brief Extract ID from a database result row
     * @param row Database result row
     * @return The ID extracted from the row
     */
    virtual auto assignId(const Database::ResultTuple &row) -> IdType = 0;

    /**
     * @brief Parse database row into struct
     * @param row Database result row
     * @return Populated StructType instance
     */
    virtual auto assignTable(const Database::ResultTuple &row) -> StructType = 0;

    /**
     * @brief Clear the buffer container
     *
     * Override if custom cleanup is needed before loading new data.
     */
    virtual void clear() { structBuffer.clear(); }

    /**
     * @brief Process a single database result row
     *
     * Extracts the ID and struct from the row and stores in buffer.
     * Override for custom row processing logic.
     *
     * @param row Database result row to process
     */
    virtual void evaluateRow(const Database::ResultTuple &row) { emplace(assignId(row), assignTable(row)); }

    /**
     * @brief Insert entry into buffer
     * @param id Entry ID
     * @param data Entry data
     */
    virtual void emplace(const IdType &id, const StructType &data) { structBuffer.emplace(id, data); }

    /**
     * @brief Remove entry from active table
     * @param id ID to remove
     * @return true if entry was removed, false if not found
     */
    auto erase(const IdType &id) -> bool { return structs.erase(id) > 0; }

    /**
     * @brief Get mutable reference to entry
     * @param id Entry ID
     * @return Mutable reference to entry (creates default if not found)
     */
    auto get(const IdType &id) -> StructType & { return structs[id]; }

private:
    ContainerType structs;      ///< Active data container
    ContainerType structBuffer; ///< Buffer for reloading data
    bool isBufferValid = false; ///< Whether buffer contains valid data
};

#endif
