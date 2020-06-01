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

#ifndef QUERY_ASSIGN_HPP
#define QUERY_ASSIGN_HPP

#include "db/Connection.hpp"
#include "db/Query.hpp"

#include <string>

namespace Database {
class QueryAssign {
private:
    const Connection &connection;
    std::string assignColumns;

public:
    QueryAssign(const QueryAssign &org) = delete;
    auto operator=(const QueryAssign &org) -> QueryAssign & = delete;
    template <typename T> void addAssignColumn(const std::string &column, const T &value) {
        Query::appendToStringList(assignColumns,
                                  Query::escapeAndChainKeys("", column) + " = " + connection.quote<T>(value));
    };

    void addAssignColumnNull(const std::string &column);

protected:
    explicit QueryAssign(const Connection &connection);

    auto buildQuerySegment() -> std::string &;
};
} // namespace Database

#endif
