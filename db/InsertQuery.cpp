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
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU  General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#include "db/InsertQuery.hpp"

#include <sstream>

#include "db/ConnectionManager.hpp"

using namespace Database;

InsertQuery::InsertQuery() {
    setOnlyOneTable(true);
    setHideTable(true);
}

InsertQuery::InsertQuery(const InsertQuery &org) {
}

InsertQuery::InsertQuery(const PConnection connection) : QueryColumns(connection), QueryTables(connection) {
    setOnlyOneTable(true);
    setHideTable(true);
}

InsertQuery::~InsertQuery() {
    std::vector<std::vector<std::string *> *>::iterator it;

    for (it = dataStorage.begin(); it < dataStorage.end(); it++) {
        std::vector<std::string *>::iterator it2;

        for (it2 = (*it)->begin(); it2 < (*it)->end(); it2++) {
            delete *it2;
        }

        delete *it;
    }
}

Result InsertQuery::execute() {
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
    std::vector<std::string *> *dataRow;;

    std::vector<std::vector<std::string *> *>::iterator itr;

    for (itr = dataStorage.begin(); itr < dataStorage.end(); itr++) {
        if (firstDone) {
            ss << "), (";
        } else {
            firstDone = true;
        }

        dataRow = *itr;

        if (columns != dataRow->size()) {
            throw new std::invalid_argument("Incorrect amount of data supplied.");
        }

        for (uint32_t column = 0; column < columns; column++) {
            ss << *(dataRow->at(column));

            if (column < columns-1) {
                ss << ", ";
            }

            delete dataRow->at(column);
            dataRow->at(column) = 0;
        }

        delete dataRow;
    }

    dataStorage.clear();

    ss << ");";

    setQuery(ss.str());
    return Query::execute();
}
