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

#ifndef QUERY_WHERE_HPP
#define QUERY_WHERE_HPP

#include "db/Connection.hpp"
#include "db/Query.hpp"

#include <boost/cstdint.hpp>
#include <stack>
#include <string>

namespace Database {
class QueryWhere {
private:
    const Connection &connection;
    std::stack<std::string> conditionsStack;
    std::string conditions;

public:
    QueryWhere(const QueryWhere &org) = delete;
    auto operator=(const QueryWhere &org) -> QueryWhere & = delete;
    QueryWhere(QueryWhere &&) = delete;
    auto operator=(QueryWhere &&) -> QueryWhere & = delete;
    ~QueryWhere() = default;

    template <typename T> void addEqualCondition(const std::string &column, const T &value) {
        addEqualCondition<T>("", column, value);
    };

    template <typename T> void addEqualCondition(const std::string &table, const std::string &column, const T &value) {
        conditionsStack.push(
                std::move(std::string(Query::escapeAndChainKeys(table, column) + " = " + connection.quote<T>(value))));
    };

    template <typename T> void addNotEqualCondition(const std::string &column, const T &value) {
        addNotEqualCondition<T>("", column, value);
    };

    template <typename T>
    void addNotEqualCondition(const std::string &table, const std::string &column, const T &value) {
        conditionsStack.push(
                std::move(std::string(Query::escapeAndChainKeys(table, column) + " != " + connection.quote<T>(value))));
    };

    void andConditions();
    void orConditions();

protected:
    explicit QueryWhere(const Connection &connection);

    auto buildQuerySegment() -> std::string;

private:
    void mergeConditions(const std::string &operation);
};
} // namespace Database

#endif
