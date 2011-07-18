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

#include "db/SelectQuery.hpp"

#include <sstream>

#include "db/ConnectionManager.hpp"

using namespace Database;

SelectQuery::SelectQuery() {
    SelectQuery(ConnectionManager::getInstance().getConnection());
    isDistinct = false;
}

SelectQuery::SelectQuery(const SelectQuery &org) : QueryColumns(org), QueryTables(org), QueryWhere(org) {
    orderBy = org.orderBy;
    isDistinct = org.isDistinct;
}

SelectQuery::SelectQuery(const PConnection connection) : QueryColumns(connection), QueryTables(connection), QueryWhere(connection) {
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
    case DESC:
        orderBy += "DESC";
    }
}

void SelectQuery::setDistinct(const bool &distinct) {
    isDistinct = distinct;
}

Result SelectQuery::execute() {
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
