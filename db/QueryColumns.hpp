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

#ifndef _QUERY_COLUMNS_HPP_
#define _QUERY_COLUMNS_HPP_

#include <string>

#include "db/Connection.hpp"
#include "db/Query.hpp"

namespace Database {
class QueryColumns : public virtual Query {
private:
    std::string columns;

public:
    virtual void addColumn(const std::string &column);
    virtual void addColumn(const std::string &table, const std::string &column);

protected:
    QueryColumns();
    QueryColumns(const QueryColumns &org);
    QueryColumns(const PConnection connection);

    virtual ~QueryColumns();

    std::string &buildQuerySegment();
};
}

#endif // _QUERY_COLUMNS_HPP_
