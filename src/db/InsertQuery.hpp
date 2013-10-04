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
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU  General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _INSERT_QUERY_HPP_
#define _INSERT_QUERY_HPP_

#include <map>
#include <string>
#include <vector>
#include <stdexcept>

#include <boost/cstdint.hpp>

#include "db/Connection.hpp"
#include "db/Result.hpp"
#include "db/QueryColumns.hpp"
#include "db/QueryTables.hpp"
#include "db/Query.hpp"

namespace Database {
class InsertQuery : Query, public QueryColumns, public QueryTables {
private:
    std::vector<std::vector<std::string *>*> dataStorage;

public:
    enum MapInsertMode {
        onlyKeys, onlyValues, keysAndValues
    };

    static const uint32_t FILL = UINT32_C(0xFFFFFFFF);

    InsertQuery();
    InsertQuery(const PConnection connection);
    InsertQuery(const InsertQuery &org) = delete;
    InsertQuery &operator=(const InsertQuery &org) = delete;
    virtual ~InsertQuery();

    template <typename T> void addValue(const QueryColumns::columnIndex &column, const T &value) throw(std::invalid_argument) {
        addValues(column, value, 1);
    };

    template <typename T> void addValues(const QueryColumns::columnIndex &column, const T &value, uint32_t count) throw(std::invalid_argument)  {
        if (count == 0) {
            return;
        }

        uint32_t columns = getColumnCount();

        if (columns <= column) {
            throw std::invalid_argument("Column index out of range.");
        }

        std::string strValue = quote<T>(value);
        std::vector<std::string *> *dataRow;

        if (!dataStorage.empty()) {
            for (const auto &dataRow : dataStorage) {
                dataRow->reserve(columns);

                if (!dataRow->at(column)) {
                    dataRow->at(column) = new std::string(strValue);

                    if (count <= 1) {
                        ;
                        return;
                    } else if (count != FILL) {
                        count--;
                    }
                }
            }
        }

        if (count == FILL) {
            return;
        }

        while (count-- > 0) {
            dataRow = new std::vector<std::string *>(columns, 0);
            dataStorage.push_back(dataRow);
            dataRow->at(column) = new std::string(strValue);
        }
    };

    template <typename T> void addValues(const QueryColumns::columnIndex &column, std::vector<T> &values) throw(std::invalid_argument) {
        for (const auto &value : values) {
            addValue<T>(column, value);
        }
    };

    template <typename Key, typename T>
    void addValues(const QueryColumns::columnIndex &column, std::map<Key,T> &values,
                   MapInsertMode mode = keysAndValues) throw(std::invalid_argument) {
        addValues<Key, T, std::less<Key> >(column, values);
    };

    template <typename Key, typename T, class Compare>
    void addValues(const QueryColumns::columnIndex &column,
                   std::map<Key,T,Compare> &values,
                   MapInsertMode mode = keysAndValues) throw(std::invalid_argument) {
        addValues<Key, T, Compare, std::allocator<std::pair<const Key, T> > >(column, values);
    };

    template <typename Key, typename T, class Compare,class Allocator>
    void addValues(const QueryColumns::columnIndex &column,
                   std::map<Key,T,Compare, Allocator> &values,
                   MapInsertMode mode = keysAndValues) throw(std::invalid_argument) {
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
    };

    virtual Result execute() override;
};
}

#endif // _INSERT_QUERY_HPP_
