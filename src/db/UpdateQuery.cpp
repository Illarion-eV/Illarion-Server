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

#include "db/UpdateQuery.hpp"

#include "db/ConnectionManager.hpp"

#include <sstream>
#include <stdexcept>

using namespace Database;

UpdateQuery::UpdateQuery() : QueryAssign(*Query::getConnection()), QueryWhere(*Query::getConnection()) {
    setOnlyOneTable(true);
}

UpdateQuery::UpdateQuery(const PConnection &connection)
        : Query(connection), QueryAssign(*Query::getConnection()), QueryWhere(*connection) {
    setOnlyOneTable(true);
};

auto UpdateQuery::execute() -> Result {
    std::stringstream ss;
    ss << "UPDATE ";
    ss << QueryTables::buildQuerySegment();
    ss << " SET ";
    ss << QueryAssign::buildQuerySegment();

    const std::string whereSegment = QueryWhere::buildQuerySegment();
    ss << whereSegment;
    ss << ";";

    setQuery(ss.str());
    return Query::execute();
}
