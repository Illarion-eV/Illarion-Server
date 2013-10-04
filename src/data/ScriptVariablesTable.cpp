/*
 * Illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of Illarionserver.
 *
 * Illarionserver  is  free  software:  you can redistribute it and/or modify it
 * under the terms of the  GNU  General  Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * Illarionserver is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY;  without  even  the  implied  warranty  of  MERCHANTABILITY  or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU  General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#include "data/ScriptVariablesTable.hpp"
#include <iostream>
#include "db/Connection.hpp"
#include "db/ConnectionManager.hpp"
#include "db/DeleteQuery.hpp"
#include "db/InsertQuery.hpp"

std::string ScriptVariablesTable::getTableName() {
    return "scriptvariables";
}

std::vector<std::string> ScriptVariablesTable::getColumnNames() {
    return {
        "svt_ids",
        "svt_string"
    };
}

std::string ScriptVariablesTable::assignId(const Database::ResultTuple &row) {
    return row["svt_ids"].as<std::string>();
}

std::string ScriptVariablesTable::assignTable(const Database::ResultTuple &row) {
    return row["svt_string"].as<std::string>();;
}

bool ScriptVariablesTable::find(const std::string &id, std::string &ret) {
    if (exists(id)) {
        ret = (*this)[id];
        return true;
    } else {
        ret = "";
        return false;
    }
}

void ScriptVariablesTable::set(const std::string &id, const std::string &value) {
    get(id) = value;
}

void ScriptVariablesTable::set(const std::string &id, int32_t value) {
    std::stringstream ss;
    ss << value;
    set(id, ss.str());
}

bool ScriptVariablesTable::remove(const std::string &id) {
    return erase(id);
}

void ScriptVariablesTable::save() {
    try {
        using namespace Database;
        PConnection connection = ConnectionManager::getInstance().getConnection();
        connection->beginTransaction();

        DeleteQuery delQuery(connection);
        delQuery.setServerTable("scriptvariables");
        delQuery.execute();

        InsertQuery insQuery(connection);
        insQuery.setServerTable("scriptvariables");
        const InsertQuery::columnIndex idColumn = insQuery.addColumn("svt_ids");
        const InsertQuery::columnIndex valueColumn = insQuery.addColumn("svt_string");

        for (const auto &var : *this) {
            if (var.second.length() > 0) {
                insQuery.addValue<std::string>(idColumn, var.first);
                insQuery.addValue<std::string>(valueColumn, var.second);
            }
        }

        insQuery.execute();

        connection->commitTransaction();
    } catch (std::exception &e) {
        std::cerr << "exception while saving script variables: " << e.what() << std::endl;
    }
}

bool ScriptVariablesTable::reloadBuffer() {
    if (first) {
        if (Base::reloadBuffer()) {
            Base::activateBuffer();
            first = false;
        } else {
            return false;
        }
    } else {
        save();
    }

    return true;
}

void ScriptVariablesTable::activateBuffer() {
}

