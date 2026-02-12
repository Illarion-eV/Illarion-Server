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
/**
 * @brief Builder class for constructing and executing SQL INSERT queries.
 * 
 * InsertQuery provides a type-safe builder for INSERT statements with support for:
 * - Multiple rows in a single INSERT
 * - Column specification (via QueryColumns mixin)
 * - Table specification (via QueryTables mixin)
 * - Type-safe value insertion
 * - Batch value insertion from vectors and maps
 * 
 * The class uses a column-based approach where values are added by column index,
 * and rows are automatically created as needed.
 * 
 * Example usage:
 * @code
 * InsertQuery query;
 * query.addColumn("name").addColumn("level");
 * query.addServerTable("chars");
 * query.addValue(0, "Alice");
 * query.addValue(1, 5);
 * query.addValue(0, "Bob");
 * query.addValue(1, 10);
 * query.execute(); // INSERT INTO chars (name, level) VALUES ('Alice', 5), ('Bob', 10);
 * @endcode
 * 
 * @note InsertQuery is movable but not copyable
 * @see Query
 * @see QueryColumns
 * @see QueryTables
 */
class InsertQuery : Query, public QueryColumns, public QueryTables {
private:
    std::vector<std::vector<std::optional<std::string>>> dataStorage; ///< Row-major storage of quoted values

public:
    /**
     * @brief Mode for inserting map data into columns.
     */
    enum MapInsertMode { 
        onlyKeys,      ///< Insert only map keys
        onlyValues,    ///< Insert only map values
        keysAndValues  ///< Insert keys in one column, values in next
    };

    static const uint32_t FILL = UINT32_C(0xFFFFFFFF); ///< Special count value to fill all existing rows

    /**
     * @brief Creates an INSERT query with auto-acquired connection.
     */
    InsertQuery();
    
    /**
     * @brief Creates an INSERT query with specified connection.
     * 
     * @param connection Database connection to use
     */
    explicit InsertQuery(const PConnection &connection);
    
    InsertQuery(const InsertQuery &org) = delete;
    auto operator=(const InsertQuery &org) -> InsertQuery & = delete;
    InsertQuery(InsertQuery &&) = default;
    auto operator=(InsertQuery &&) -> InsertQuery & = default;
    ~InsertQuery() override = default;

    /**
     * @brief Adds a single value to a column.
     * 
     * Creates a new row with this value if all existing rows have values for this column.
     * 
     * @tparam T Type of value
     * @param column Column index (0-based)
     * @param value Value to insert
     * @throws std::invalid_argument if column index out of range
     */
    template <typename T> void addValue(const QueryColumns::columnIndex &column, const T &value) {
        addValues(column, value, 1);
    }

    /**
     * @brief Adds the same value to a column for multiple rows.
     * 
     * First fills existing empty cells in this column, then creates new rows as needed.
     * 
     * @tparam T Type of value
     * @param column Column index (0-based)
     * @param value Value to insert
     * @param count Number of rows to insert (FILL to fill all existing rows only)
     * @throws std::invalid_argument if column index out of range
     */
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

    /**
     * @brief Adds values from a vector, one per row.
     * 
     * @tparam T Type of values
     * @param column Column index
     * @param values Vector of values to insert
     */
    template <typename T> void addValues(const QueryColumns::columnIndex &column, const std::vector<T> &values) {
        for (const auto &value : values) {
            addValue<T>(column, value);
        }
    }

    /**
     * @brief Adds values from a map (keys, values, or both).
     * 
     * @tparam Key Map key type
     * @tparam T Map value type
     * @param column Starting column index
     * @param values Map to insert from
     * @param mode How to insert map data (keys, values, or both)
     */
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

    /**
     * @brief Builds and executes the INSERT query.
     * 
     * @return Query result (typically empty for INSERT)
     */
    auto execute() -> Result override;
};
} // namespace Database

#endif
