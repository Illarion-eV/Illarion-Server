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

#include "db/QueryColumns.hpp"
#include "db/Query.hpp"

using namespace Database;

QueryColumns::QueryColumns() {
    hideTable = false;
    nextColumn = 0;
};

auto QueryColumns::addColumn(const std::string &column) -> QueryColumns::columnIndex {
    Query::appendToStringList(columns, Query::escapeKey(column));
    return nextColumn++;
}

auto QueryColumns::addColumn(const std::string &table, const std::string &column) -> QueryColumns::columnIndex {
    if (hideTable) {
        return addColumn(column);
    }

    Query::appendToStringList(columns, Query::escapeAndChainKeys(table, column));
    return nextColumn++;
}

auto QueryColumns::buildQuerySegment() -> std::string & {
    return columns;
}

auto QueryColumns::getColumnCount() const -> uint32_t {
    return (uint32_t) nextColumn;
}

void QueryColumns::setHideTable(bool hide) {
    hideTable = hide;
}
