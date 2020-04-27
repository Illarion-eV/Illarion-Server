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

#ifndef _QUERY_COLUMNS_HPP_
#define _QUERY_COLUMNS_HPP_

#include <string>
#include <boost/cstdint.hpp>

#include "db/Connection.hpp"

namespace Database {
class QueryColumns {
public:
    typedef uint8_t columnIndex;

private:
    std::string columns;
    bool hideTable;
    columnIndex nextColumn;

public:

    columnIndex addColumn(const std::string &column);
    columnIndex addColumn(const std::string &table, const std::string &column);

    void addColumns() {}
    template<typename... Args>
    void addColumns(const std::string &column, const Args &... args) {
        addColumn(column);
        addColumns(args...);
    }

    void addColumnsWithTable(const std::string &table) {}
    template<typename... Args>
    void addColumnsWithTable(const std::string &table, const std::string &column, const Args &... args) {
        addColumn(table, column);
        addColumnsWithTable(table, args...);
    }

protected:
    QueryColumns();
    QueryColumns(const QueryColumns &org) = delete;
    QueryColumns &operator=(const QueryColumns &org) = delete;

    std::string &buildQuerySegment();
    uint32_t getColumnCount() const;

    void setHideTable(const bool hide);
};
}

#endif // _QUERY_COLUMNS_HPP_
