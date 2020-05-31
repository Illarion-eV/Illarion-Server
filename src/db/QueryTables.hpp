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

#ifndef QUERY_TABLES_HPP
#define QUERY_TABLES_HPP

#include <string>
#include <stdexcept>

#include "db/Connection.hpp"

namespace Database {
class QueryTables {
private:
    std::string tables;
    bool oneTable;

public:
    QueryTables(const QueryTables &org) = delete;
    auto operator=(const QueryTables &org) -> QueryTables & = delete;

    void addServerTable(const std::string &table);
    void addAccountTable(const std::string &table);

    void setServerTable(const std::string &table);
    void setAccountTable(const std::string &table);
protected:
    QueryTables();
    
    void setOnlyOneTable(const bool &enabled);
    auto buildQuerySegment() -> std::string &;
};
}

#endif
