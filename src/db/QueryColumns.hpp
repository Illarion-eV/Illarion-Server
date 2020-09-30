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

#ifndef QUERY_COLUMNS_HPP
#define QUERY_COLUMNS_HPP

#include "db/Connection.hpp"

#include <boost/cstdint.hpp>
#include <string>

namespace Database {
class QueryColumns {
public:
    using columnIndex = uint8_t;

    QueryColumns(const QueryColumns &org) = delete;
    auto operator=(const QueryColumns &org) -> QueryColumns & = delete;
    QueryColumns(QueryColumns &&) = default;
    auto operator=(QueryColumns &&) -> QueryColumns & = default;
    ~QueryColumns() = default;

private:
    std::string columns;
    bool hideTable{false};
    columnIndex nextColumn{0};

public:
    auto addColumn(const std::string &column) -> columnIndex;
    auto addColumn(const std::string &table, const std::string &column) -> columnIndex;

    static void addColumns() {}
    template <typename... Args> void addColumns(const std::string &column, const Args &... args) {
        addColumn(column);
        addColumns(args...);
    }

    static void addColumnsWithTable(const std::string &table) {}
    template <typename... Args>
    void addColumnsWithTable(const std::string &table, const std::string &column, const Args &... args) {
        addColumn(table, column);
        addColumnsWithTable(table, args...);
    }

protected:
    QueryColumns() = default;

    [[nodiscard]] auto buildQuerySegment() const -> const std::string &;
    [[nodiscard]] auto getColumnCount() const -> uint32_t;

    void setHideTable(bool hide);
};
} // namespace Database

#endif
