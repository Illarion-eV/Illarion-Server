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

#include "db/DeleteQuery.hpp"

#include "db/ConnectionManager.hpp"

#include <sstream>

using namespace Database;

DeleteQuery::DeleteQuery() : QueryWhere(*Query::getConnection()) { setOnlyOneTable(true); }

DeleteQuery::DeleteQuery(const PConnection &connection) : Query(connection), QueryWhere(*connection) {
    setOnlyOneTable(true);
}

auto DeleteQuery::execute() -> Result {
    std::stringstream ss;
    ss << "DELETE FROM ";
    ss << QueryTables::buildQuerySegment();
    const std::string whereSegment = QueryWhere::buildQuerySegment();
    ss << whereSegment;
    ss << ";";

    setQuery(ss.str());
    return Query::execute();
}
