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

#include "db/UpdateQuery.hpp"

#include <sstream>
#include <stdexcept>

#include "db/ConnectionManager.hpp"

using namespace Database;

UpdateQuery::UpdateQuery() {
    setOnlyOneTable(true);
}

UpdateQuery::UpdateQuery(const UpdateQuery &org) {
};

UpdateQuery::UpdateQuery(const PConnection connection) : QueryAssign(connection), QueryTables(connection), QueryWhere(connection) {
    setOnlyOneTable(true);
};

Result UpdateQuery::execute() {
    std::stringstream ss;
    ss << "UPDATE ";
    ss << QueryTables::buildQuerySegment();
    ss << " SET ";
    ss << QueryAssign::buildQuerySegment();

    const std::string whereSegment = QueryWhere::buildQuerySegment();
#if !defined(UPDATE_QUERY_NO_WHERE_CHECK)
    if (whereSegment.empty()) {
        throw new std::logic_error("Update Query without WHERE is not acceptable");
    }
#endif
    ss << whereSegment;
    ss << ";";

    setQuery(ss.str());
    return Query::execute();
}
