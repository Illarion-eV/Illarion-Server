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

#ifndef _DELETE_QUERY_HPP_
#define _DELETE_QUERY_HPP_

#include "db/Connection.hpp"
#include "db/Result.hpp"
#include "db/QueryTables.hpp"
#include "db/QueryWhere.hpp"

namespace Database {
class DeleteQuery : Query, public QueryTables, public QueryWhere {
public:
    DeleteQuery();
    DeleteQuery(const PConnection connection);
    DeleteQuery(const DeleteQuery &org) = delete;
    DeleteQuery &operator=(const DeleteQuery &org) = delete;

    virtual Result execute();
};

}

#endif // _DELETE_QUERY_HPP_
