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

#ifndef INSERT_QUERY_HPP
#define INSERT_QUERY_HPP

#include "db/Connection.hpp"
#include "db/Query.hpp"
#include "db/QueryColumns.hpp"
#include "db/QueryTables.hpp"
#include "db/Result.hpp"

#include <boost/cstdint.hpp>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace Database {
class InsertQuery : Query, public QueryColumns, public QueryTables {
private:
    std::vector<std::vector<std::optional<std::string>>> dataStorage;

public:
    enum MapInsertMode { onlyKeys, onlyValues, keysAndValues };

    static const uint32_t FILL = UINT32_C(0xFFFFFFFF);

    InsertQuery();
    explicit InsertQuery(const PConnection &connection);
    InsertQuery(const InsertQuery &org) = delete;
    auto operator=(const InsertQuery &org) -> InsertQuery & = delete;
    InsertQuery(InsertQuery &&) = default;
    auto operator=(InsertQuery &&) -> InsertQuery & = default;
    ~InsertQuery() override = default;

    template <typename T> void addValue(const QueryColumns::columnIndex &column, const T &value) {
        addValues(column, value, 1);
    }

    template <typename T> void addValues(const QueryColumns::columnIndex &column, const T &value, uint32_t count) {
        if (count == 0) {
            return;
        }

        uint32_t columns = getColumnCount();

        if (columns <= column) {
            throw std::invalid_argument("Column index out of range.");
        }

        std::string strValue = quote<T>(value);

        if (!dataStorage.empty()) {
            for (auto &dataRow : dataStorage) {
                dataRow.reserve(columns);

                if (!dataRow.at(column)) {
                    dataRow.at(column) = strValue;

                    if (count <= 1) {
                        return;
                    }
                    if (count != FILL) {
                        count--;
                    }
                }
            }
        }

        if (count == FILL) {
            return;
        }

        while (count-- > 0) {
            dataStorage.emplace_back(columns, std::nullopt);
            dataStorage.back().at(column) = strValue;
        }
    }

    template <typename T> void addValues(const QueryColumns::columnIndex &column, const std::vector<T> &values) {
        for (const auto &value : values) {
            addValue<T>(column, value);
        }
    }

    template <typename Key, typename T>
    void addValues(const QueryColumns::columnIndex &column, const std::map<Key, T> &values,
                   MapInsertMode mode = keysAndValues) {
        addValues<Key, T, std::less<Key>>(column, values);
    }

    template <typename Key, typename T, class Compare>
    void addValues(const QueryColumns::columnIndex &column, const std::map<Key, T, Compare> &values,
                   MapInsertMode mode = keysAndValues) {
        addValues<Key, T, Compare, std::allocator<std::pair<const Key, T>>>(column, values);
    }

    template <typename Key, typename T, class Compare, class Allocator>
    void addValues(const QueryColumns::columnIndex &column, const std::map<Key, T, Compare, Allocator> &values,
                   MapInsertMode mode = keysAndValues) {
        for (const auto &key_value : values) {
            switch (mode) {
            case onlyKeys:
                addValue<Key>(column, key_value.first);
                break;

            case onlyValues:
                addValue<T>(column, key_value.second);
                break;

            case keysAndValues:
                addValue<Key>(column, key_value.first);
                addValue<T>(column + 1, key_value.second);
                break;
            }
        }
    }

    auto execute() -> Result override;
};
} // namespace Database

#endif
