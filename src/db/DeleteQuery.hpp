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
class DeleteQuery : Query, public QueryTables, public QueryWhere {
public:
    DeleteQuery();
    explicit DeleteQuery(const PConnection &connection);
    DeleteQuery(const DeleteQuery &org) = delete;
    auto operator=(const DeleteQuery &org) -> DeleteQuery & = delete;
    DeleteQuery(DeleteQuery &&) = delete;
    auto operator=(DeleteQuery &&) -> DeleteQuery & = delete;
    ~DeleteQuery() override = default;

    auto execute() -> Result override;
};

} // namespace Database

#endif
