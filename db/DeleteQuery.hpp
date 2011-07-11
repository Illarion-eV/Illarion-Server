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

#include <string>
#include <stack>

#include <boost/cstdint.hpp>

#include "db/Connection.hpp"
#include "db/Result.hpp"
#include "db/Query.hpp"
#include "db/QueryWhere.hpp"

namespace Database {
class DeleteQuery : public QueryWhere {
private:
    std::string table;
public:
    DeleteQuery();
    DeleteQuery(const SelectQuery &org);
    DeleteQuery(const PConnection connection);
    ~DeleteQuery();

    void setServerTable(const std::string &table);
    void setAccountTable(const std::string &table);

    virtual Result execute();
};

}

#endif // _DELETE_QUERY_HPP_
