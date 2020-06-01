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

#include "db/SelectQuery.hpp"

#include "db/ConnectionManager.hpp"

#include <sstream>

using namespace Database;

SelectQuery::SelectQuery() : QueryWhere(*Query::getConnection()) {
    setOnlyOneTable(false);
    isDistinct = false;
}

SelectQuery::SelectQuery(const PConnection &connection) : Query(connection), QueryWhere(*connection) {
    setOnlyOneTable(false);
    isDistinct = false;
};

void SelectQuery::addOrderBy(const std::string &column, const OrderDirection &dir) {
    addOrderBy("", column, dir);
}

void SelectQuery::addOrderBy(const std::string &table, const std::string &column, const OrderDirection &dir) {
    if (!orderBy.empty()) {
        orderBy += ", ";
    }

    orderBy += escapeAndChainKeys(table, column);
    orderBy += " ";

    switch (dir) {
    case ASC:
        orderBy += "ASC";
        break;

    case DESC:
        orderBy += "DESC";
        break;
    }
}

void SelectQuery::setDistinct(const bool &distinct) {
    isDistinct = distinct;
}

auto SelectQuery::execute() -> Result {
    std::stringstream ss;
    ss << "SELECT ";

    if (isDistinct) {
        ss << "DISTINCT ";
    }

    ss << QueryColumns::buildQuerySegment();
    ss << " FROM ";
    ss << QueryTables::buildQuerySegment();
    ss << QueryWhere::buildQuerySegment();

    if (!orderBy.empty()) {
        ss << " ORDER BY " << orderBy;
    }

    ss << ";";

    setQuery(ss.str());
    return Query::execute();
}
