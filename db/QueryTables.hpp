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

#ifndef _QUERY_TABLES_HPP_
#define _QUERY_TABLES_HPP_

#include <string>
#include <stdexcept>

#include "db/Connection.hpp"
#include "db/Query.hpp"

namespace Database {
class QueryTables : public virtual Query {
private:
    std::string tables;
    bool oneTable;

public:
    void addServerTable(const std::string &table) throw(std::logic_error);
    void addAccountTable(const std::string &table) throw(std::logic_error);

    void setServerTable(const std::string &table);
    void setAccountTable(const std::string &table);
protected:
    QueryTables();

    void setOnlyOneTable(const bool &enabled);
    std::string &buildQuerySegment();
private:
    QueryTables(const QueryTables &org);
};
}

#endif // _QUERY_TABLES_HPP_
