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

#ifndef SELECT_QUERY_HPP
#define SELECT_QUERY_HPP

#include "db/Connection.hpp"
#include "db/Query.hpp"
#include "db/QueryColumns.hpp"
#include "db/QueryTables.hpp"
#include "db/QueryWhere.hpp"
#include "db/Result.hpp"

#include <boost/cstdint.hpp>
#include <string>

namespace Database {
/**
 * @brief Builder class for constructing and executing SQL SELECT queries.
 * 
 * SelectQuery provides a type-safe builder pattern for SELECT statements with support for:
 * - Column selection (via QueryColumns mixin)
 * - Table joins (via QueryTables mixin)
 * - WHERE conditions (via QueryWhere mixin)
 * - ORDER BY clauses with direction
 * - DISTINCT keyword
 * 
 * Example usage:
 * @code
 * SelectQuery query;
 * query.addColumn("name").addColumn("level");
 * query.addServerTable("chars");
 * query.addWhereClause("char_id", "=", 123);
 * query.addOrderBy("name", SelectQuery::ASC);
 * auto result = query.execute();
 * @endcode
 * 
 * @note SelectQuery is non-copyable and non-movable
 * @see Query
 * @see QueryColumns
 * @see QueryTables
 * @see QueryWhere
 */
class SelectQuery : Query, public QueryColumns, public QueryTables, public QueryWhere {
private:
    std::string orderBy; ///< ORDER BY clause string
    bool isDistinct{false}; ///< Whether to use DISTINCT keyword

public:
    /**
     * @brief Sort direction for ORDER BY clauses.
     */
    enum OrderDirection { 
        ASC,  ///< Ascending order
        DESC  ///< Descending order
    };

    /**
     * @brief Creates a SELECT query with auto-acquired connection.
     */
    SelectQuery();
    
    /**
     * @brief Creates a SELECT query with specified connection.
     * 
     * @param connection Database connection to use
     */
    explicit SelectQuery(const PConnection &connection);
    
    SelectQuery(const SelectQuery &org) = delete;
    auto operator=(const SelectQuery &org) -> SelectQuery & = delete;
    SelectQuery(SelectQuery &&) = delete;
    auto operator=(SelectQuery &&) -> SelectQuery & = delete;
    ~SelectQuery() override = default;

    /**
     * @brief Adds an ORDER BY clause for a column.
     * 
     * @param column Column name to sort by
     * @param dir Sort direction (ASC or DESC)
     */
    void addOrderBy(const std::string &column, const OrderDirection &dir);
    
    /**
     * @brief Adds an ORDER BY clause for a table-qualified column.
     * 
     * @param table Table name
     * @param column Column name to sort by
     * @param dir Sort direction (ASC or DESC)
     */
    void addOrderBy(const std::string &table, const std::string &column, const OrderDirection &dir);

    /**
     * @brief Enables or disables DISTINCT keyword in SELECT.
     * 
     * @param distinct true to use SELECT DISTINCT, false for SELECT
     */
    void setDistinct(const bool &distinct);

    /**
     * @brief Builds and executes the SELECT query.
     * 
     * Constructs the SQL from all configured clauses and executes it.
     * 
     * @return Query result set
     */
    auto execute() -> Result override;
};
} // namespace Database

#endif
