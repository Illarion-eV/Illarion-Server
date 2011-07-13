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

namespace Database {
class InsertQuery : public QueryColumns, public QueryTables {
private:
    std::vector<std::vector<std::string*>*> dataStorage;
    
public:
    enum MapInsertMode {
        onlyKeys, onlyValues, keysAndValues
    };
    
    InsertQuery();
    InsertQuery(const InsertQuery &org);
    InsertQuery(const PConnection connection);
    virtual ~InsertQuery();

    template <typename T> void addValue(const columnIndex &column, const T &value) throw (std::invalid_argument);
    template <typename T> void addValues(const columnIndex &column, std::vector<T> &values) throw (std::invalid_argument);
    template <typename Key, typename T>
        void addValues(const columnIndex &column, std::map<Key,T> &values,
                       MapInsertMode mode = keysAndValues) throw (std::invalid_argument);
    template <typename Key, typename T, class Compare>
        void addValues(const columnIndex &column,
                       std::map<Key,T,Compare> &values,
                       MapInsertMode mode = keysAndValues) throw (std::invalid_argument);
    template <typename Key, typename T, class Compare,class Allocator>
        void addValues(const columnIndex &column,
                       std::map<Key,T,Compare, Allocator> &values,
                       MapInsertMode mode = keysAndValues) throw (std::invalid_argument);
    
    virtual Result execute();
};
}

#endif // _INSERT_QUERY_HPP_
