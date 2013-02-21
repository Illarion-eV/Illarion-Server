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

#ifndef _QUERY_ASSIGN_HPP_
#define _QUERY_ASSIGN_HPP_

#include <string>

#include "db/Connection.hpp"
#include "db/Query.hpp"

namespace Database {
class QueryAssign {
private:
    const Connection &connection;
    std::string assignColumns;

public:
    template<typename T> void addAssignColumn(const std::string &column, const T &value) {
        Query::appendToStringList(assignColumns, Query::escapeAndChainKeys("", column) + " = " + connection.quote<T>(value));
    };

    void addAssignColumnNull(const std::string &column);
protected:
    QueryAssign(const Connection &connection);
    QueryAssign(const QueryAssign &org) = delete;
    QueryAssign &operator=(const QueryAssign &org) = delete;

    std::string &buildQuerySegment();
};
}

#endif // _QUERY_ASSIGN_HPP_
