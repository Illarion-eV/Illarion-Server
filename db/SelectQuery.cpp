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
}

SelectQuery::SelectQuery(const SelectQuery &org) : QueryColumns(org), QueryTables(org), QueryWhere(org);

SelectQuery::SelectQuery(const PConnection connection) : QueryColumns(connection), QueryTables(connection), QueryWhere(connection) {
    setOnlyOneTable(false);
};

Result SelectQuery::execute() {
    std::stringstream ss;
    ss << "SELECT ";
    ss << QueryColumns::buildQuerySegment();
    ss << " FROM ";
    ss << QueryTables::buildQuerySegment();
    ss << QueryWhere::buildQuerySegment();
    ss << ";";

    setQuery(ss.str());
    Query::execute();
}
