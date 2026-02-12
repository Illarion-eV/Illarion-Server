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

#ifndef CONTAINER_OBJECT_TABLE_HPP
#define CONTAINER_OBJECT_TABLE_HPP

#include "data/StructTable.hpp"
#include "types.hpp"

/**
 * @brief Table for container storage capacity
 *
 * Loads container data from the database "container" table, mapping item IDs
 * to their container slot capacity. Items with entries in this table can hold
 * other items, with the number of slots determining how many items can be stored.
 *
 * This is a simple table that only stores the slot count - no complex data
 * structure is needed, hence the direct use of TYPE_OF_CONTAINERSLOTS as the
 * stored value type.
 *
 * Database table: container
 * Columns: con_itemid (item ID), con_slots (number of storage slots)
 */
class ContainerObjectTable : public StructTable<TYPE_OF_ITEM_ID, TYPE_OF_CONTAINERSLOTS> {
public:
    /**
     * @brief Get database table name
     * @return "container"
     */
    auto getTableName() const -> std::string override;

    /**
     * @brief Get column names for the container table
     * @return Vector containing container table column names
     */
    auto getColumnNames() -> std::vector<std::string> override;

    /**
     * @brief Extract item ID from database row
     * @param row Database result row
     * @return Item ID from con_itemid column
     */
    auto assignId(const Database::ResultTuple &row) -> TYPE_OF_ITEM_ID override;

    /**
     * @brief Extract container slot count from database row
     * @param row Database result row
     * @return Number of slots from con_slots column
     */
    auto assignTable(const Database::ResultTuple &row) -> TYPE_OF_CONTAINERSLOTS override;
};

#endif
