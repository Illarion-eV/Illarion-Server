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
class UpdateQuery : Query, public QueryAssign, public QueryTables, public QueryWhere {
public:
    UpdateQuery();
    explicit UpdateQuery(const PConnection &connection);
    UpdateQuery(const UpdateQuery &org) = delete;
    auto operator=(const UpdateQuery &org) -> UpdateQuery & = delete;
    UpdateQuery(UpdateQuery &&) = delete;
    auto operator=(UpdateQuery &&) -> UpdateQuery & = delete;
    ~UpdateQuery() override = default;

    auto execute() -> Result override;
};
} // namespace Database

#endif
