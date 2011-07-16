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

#include "db/QueryColumns.hpp"

using namespace Database;

QueryColumns::QueryColumns() : Query() {
    hideTable = false;
    nextColumn = 0;
};

QueryColumns::QueryColumns(const QueryColumns &org) : Query(org) {
    columns = org.columns;
    hideTable = org.hideTable;
    nextColumn = nextColumn;
}

QueryColumns::QueryColumns(const PConnection connection) : Query(connection) {
    hideTable = false;
    nextColumn = 0;
}

QueryColumns::columnIndex QueryColumns::addColumn(const std::string &column) {
    appendToStringList(columns, escapeKey(column));
    return nextColumn++;
}

QueryColumns::columnIndex QueryColumns::addColumn(const std::string &table, const std::string &column) {
    if (hideTable) {
        return addColumn(column);
    }

    appendToStringList(columns, escapeAndChainKeys(table, column));
    return nextColumn++;
}

std::string &QueryColumns::buildQuerySegment() {
    return columns;
}

uint32_t &QueryColumns::getColumnCount() {
    return (uint32_t) nextColumn;
}

void setHideTable(const bool &hide) {
    hideTable = hide;
}
