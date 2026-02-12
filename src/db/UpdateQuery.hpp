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

#ifndef UPDATE_QUERY_HPP
#define UPDATE_QUERY_HPP

#include "db/Connection.hpp"
#include "db/Query.hpp"
#include "db/QueryAssign.hpp"
#include "db/QueryTables.hpp"
#include "db/QueryWhere.hpp"
#include "db/Result.hpp"

#include <boost/cstdint.hpp>
#include <string>

namespace Database {
/**
 * @brief Builder class for constructing and executing SQL UPDATE queries.
 * 
 * UpdateQuery provides a type-safe builder for UPDATE statements with support for:
 * - Column value assignment (via QueryAssign mixin)
 * - Single table updates (via QueryTables mixin)
 * - WHERE conditions (via QueryWhere mixin)
 * 
 * The class enforces single-table updates only for safety.
 * 
 * Example usage:
 * @code
 * UpdateQuery query;
 * query.addServerTable("chars");
 * query.addAssignColumn("level", 10);
 * query.addAssignColumn("experience", 5000);
 * query.addWhereClause("char_id", "=", 123);
 * query.execute(); // UPDATE chars SET level = 10, experience = 5000 WHERE char_id = 123;
 * @endcode
 * 
 * @note UpdateQuery is non-copyable and non-movable
 * @see Query
 * @see QueryAssign
 * @see QueryTables
 * @see QueryWhere
 */
class UpdateQuery : Query, public QueryAssign, public QueryTables, public QueryWhere {
public:
    /**
     * @brief Creates an UPDATE query with auto-acquired connection.
     */
    UpdateQuery();
    
    /**
     * @brief Creates an UPDATE query with specified connection.
     * 
     * @param connection Database connection to use
     */
    explicit UpdateQuery(const PConnection &connection);
    
    UpdateQuery(const UpdateQuery &org) = delete;
    auto operator=(const UpdateQuery &org) -> UpdateQuery & = delete;
    UpdateQuery(UpdateQuery &&) = delete;
    auto operator=(UpdateQuery &&) -> UpdateQuery & = delete;
    ~UpdateQuery() override = default;

    /**
     * @brief Builds and executes the UPDATE query.
     * 
     * Constructs the SQL from table, SET assignments, and WHERE clauses.
     * 
     * @return Query result (typically shows number of rows affected)
     */
    auto execute() -> Result override;
};
} // namespace Database

#endif
