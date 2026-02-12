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

#ifndef DELETE_QUERY_HPP
#define DELETE_QUERY_HPP

#include "db/Connection.hpp"
#include "db/QueryTables.hpp"
#include "db/QueryWhere.hpp"
#include "db/Result.hpp"

namespace Database {
/**
 * @brief Builder class for constructing and executing SQL DELETE queries.
 * 
 * DeleteQuery provides a type-safe builder for DELETE statements with support for:
 * - Single table deletion (via QueryTables mixin)
 * - WHERE conditions (via QueryWhere mixin)
 * 
 * The class enforces single-table deletes only for safety.
 * 
 * Example usage:
 * @code
 * DeleteQuery query;
 * query.addServerTable("old_items");
 * query.addWhereClause("created_at", "<", "2020-01-01");
 * query.execute(); // DELETE FROM old_items WHERE created_at < '2020-01-01';
 * @endcode
 * 
 * @warning Always use WHERE clauses to avoid accidentally deleting all rows
 * @note DeleteQuery is non-copyable and non-movable
 * @see Query
 * @see QueryTables
 * @see QueryWhere
 */
class DeleteQuery : Query, public QueryTables, public QueryWhere {
public:
    /**
     * @brief Creates a DELETE query with auto-acquired connection.
     */
    DeleteQuery();
    
    /**
     * @brief Creates a DELETE query with specified connection.
     * 
     * @param connection Database connection to use
     */
    explicit DeleteQuery(const PConnection &connection);
    
    DeleteQuery(const DeleteQuery &org) = delete;
    auto operator=(const DeleteQuery &org) -> DeleteQuery & = delete;
    DeleteQuery(DeleteQuery &&) = delete;
    auto operator=(DeleteQuery &&) -> DeleteQuery & = delete;
    ~DeleteQuery() override = default;

    /**
     * @brief Builds and executes the DELETE query.
     * 
     * Constructs the SQL from table and WHERE clauses.
     * 
     * @return Query result (typically shows number of rows deleted)
     */
    auto execute() -> Result override;
};

} // namespace Database

#endif
