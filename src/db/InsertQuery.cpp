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

#include "db/InsertQuery.hpp"

#include "db/ConnectionManager.hpp"

#include <sstream>

using namespace Database;

InsertQuery::InsertQuery() {
    setOnlyOneTable(true);
    setHideTable(true);
}

InsertQuery::InsertQuery(const PConnection &connection) : Query(connection) {
    setOnlyOneTable(true);
    setHideTable(true);
}

auto InsertQuery::execute() -> Result {
    if (dataStorage.empty()) {
        Result result;
        return result;
    }

    std::stringstream ss;
    ss << "INSERT INTO ";
    ss << QueryTables::buildQuerySegment();
    ss << " (";
    ss << QueryColumns::buildQuerySegment();
    ss << ") VALUES ";
    ss << "(";
    uint32_t columns = getColumnCount();
    bool firstDone = false;

    for (const auto &dataRow : dataStorage) {
        if (firstDone) {
            ss << "), (";
        } else {
            firstDone = true;
        }

        if (columns != dataRow.size()) {
            throw std::invalid_argument("Incorrect amount of data supplied.");
        }

        for (uint32_t column = 0; column < columns; column++) {
            ss << *(dataRow.at(column));

            if (column < columns - 1) {
                ss << ", ";
            }
        }
    }

    dataStorage.clear();

    ss << ");";

    setQuery(ss.str());
    return Query::execute();
}
