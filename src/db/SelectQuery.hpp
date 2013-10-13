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

#ifndef _SELECT_QUERY_HPP_
#define _SELECT_QUERY_HPP_

#include <string>

#include <boost/cstdint.hpp>

#include "db/Connection.hpp"
#include "db/Result.hpp"
#include "db/Query.hpp"
#include "db/QueryColumns.hpp"
#include "db/QueryTables.hpp"
#include "db/QueryWhere.hpp"

namespace Database {
class SelectQuery : Query, public QueryColumns, public QueryTables, public QueryWhere {
private:
    std::string orderBy;
    bool isDistinct;
public:
    enum OrderDirection {
        ASC, DESC
    };

    SelectQuery();
    SelectQuery(const PConnection connection);
    SelectQuery(const SelectQuery &org) = delete;
    SelectQuery &operator=(const SelectQuery &org) = delete;

    void addOrderBy(const std::string &column, const OrderDirection &dir);
    void addOrderBy(const std::string &table, const std::string &column, const OrderDirection &dir);

    void setDistinct(const bool &distinct);

    virtual Result execute() override;
};
}

#endif // _SELECT_QUERY_HPP_
