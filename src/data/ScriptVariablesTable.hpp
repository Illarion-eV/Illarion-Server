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

class ScriptVariablesTable : public StructTable<std::string, std::string> {
public:
    auto getTableName() -> std::string override;
    auto getColumnNames() -> std::vector<std::string> override;
    auto assignId(const Database::ResultTuple &row) -> std::string override;
    auto assignTable(const Database::ResultTuple &row) -> std::string override;

    auto find(const std::string &id, std::string &ret) -> bool;
    void set(const std::string &id, const std::string &value);
    void set(const std::string &id, int32_t value);
    auto remove(const std::string &id) -> bool;

    void save();

    auto reloadBuffer() -> bool override;
    void activateBuffer() override;

private:
    using Base = StructTable<std::string, std::string>;
    bool first = true;
};

#endif

