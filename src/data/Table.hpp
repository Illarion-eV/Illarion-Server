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

#ifndef TABLE_HPP
#define TABLE_HPP

/**
 * @brief Abstract base class for all game data tables
 *
 * This interface defines the contract for table classes that load and manage
 * game data from the database. Tables support double-buffering to allow safe
 * reloading of data without interrupting gameplay.
 *
 * The typical reload workflow is:
 * 1. Call reloadBuffer() to load new data into a buffer
 * 2. Call reloadScripts() to refresh any associated script bindings
 * 3. Call activateBuffer() to atomically swap the buffer with active data
 */
class Table {
public:
    /**
     * @brief Reload table data from database into a buffer
     *
     * Loads fresh data from the database into an internal buffer without
     * affecting the currently active data. This allows validation and
     * preparation before activation.
     *
     * @return true if reload was successful, false on error
     */
    virtual auto reloadBuffer() -> bool = 0;

    /**
     * @brief Reload script bindings associated with this table
     *
     * Refreshes any Lua script connections or callbacks that depend on
     * this table's data. Called after reloadBuffer() but before activateBuffer().
     */
    virtual void reloadScripts() = 0;

    /**
     * @brief Activate the buffered data, making it live
     *
     * Atomically swaps the buffered data with the active data. After this
     * call, the newly loaded data becomes active and the old data is discarded.
     */
    virtual void activateBuffer() = 0;

    Table() = default;
    virtual ~Table() = default;
    Table(const Table &) = default;
    auto operator=(const Table &) -> Table & = default;
    Table(Table &&) = default;
    auto operator=(Table &&) -> Table & = default;
};

#endif
