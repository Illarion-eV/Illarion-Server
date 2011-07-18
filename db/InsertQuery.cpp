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

#include "db/InsertQuery.hpp"

#include <sstream>

#include "db/ConnectionManager.hpp"

using namespace Database;

InsertQuery::InsertQuery() {
    InsertQuery(ConnectionManager::getInstance().getConnection());
}

InsertQuery::InsertQuery(const InsertQuery &org) : QueryColumns(org), QueryTables(org) {
};

InsertQuery::InsertQuery(const PConnection connection) : QueryColumns(connection), QueryTables(connection) {
    setOnlyOneTable(true);
    setHideTable(true);
};

template <typename T> void InsertQuery::addValue(const QueryColumns::columnIndex &column, const T &value) throw(std::invalid_argument) {
    addValues(column, value, 1);
}

template <typename T> void InsertQuery::addValues(const QueryColumns::columnIndex &column, const T &value, uint32_t count) throw(std::invalid_argument) {
    if (count == 0) {
        return;
    }

    uint32_t columns = getColumnCount();

    if (columns <= column) {
        throw new std::invalid_argument("Column index out of range.");
    }

    std::string strValue = quote<T>(value);
    std::vector<std::string *> *dataRow;

    if (!dataStorage.empty()) {
        for (std::vector<std::vector<std::string *> *>::iterator itr = dataStorage.begin(); itr < dataStorage.end(); itr++) {
            dataRow = *itr;
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
}

template <typename T> void InsertQuery::addValues(const QueryColumns::columnIndex &column, std::vector<T> &values) throw(std::invalid_argument) {
    typename std::vector<T>::iterator itr;
    for (itr = values.begin(); itr < values.end(); itr++) {
        addValue<T>(column, *itr);
    }
}

template <typename Key, typename T, class Compare, class Allocator>
void InsertQuery::addValues(const QueryColumns::columnIndex &column,
               std::map<Key, T, Compare, Allocator> &values,
               MapInsertMode mode) throw(std::invalid_argument) {
    typename std::map<Key, T, Compare, Allocator>::iterator itr;
    for (itr = values.begin(); itr < values.end(); itr++) {
        switch (mode) {
        case onlyKeys:
            addValue<Key>(column, itr->first);
            break;
        case onlyValues:
            addValue<T>(column, itr->second);
            break;
        case keysAndValues:
            addValue<Key>(column, itr->first);
            addValue<T>(column + 1, itr->second);
            break;
        }
    }
}

template <typename Key, typename T, class Compare>
void InsertQuery::addValues(const QueryColumns::columnIndex &column,
               std::map<Key, T, Compare> &values,
               MapInsertMode mode) throw(std::invalid_argument) {
    addValues<Key, T, Compare, std::allocator<std::pair<const Key, T> > >(column, values);
}

template <typename Key, typename T>
void InsertQuery::addValues(const QueryColumns::columnIndex &column,
               std::map<Key, T> &values,
               MapInsertMode mode) throw(std::invalid_argument) {
    addValues<Key, T, std::less<Key> >(column, values);
}

Result InsertQuery::execute() {
    std::stringstream ss;
    ss << "INSERT INTO ";
    ss << QueryTables::buildQuerySegment();
    ss << " (";
    ss << QueryColumns::buildQuerySegment();
    ss << ") VALUES ";

    if (dataStorage.empty()) {
        throw new std::logic_error("There is no data to insert!");
    }

    ss << "(";
    uint32_t columns = getColumnCount();
    bool firstDone = false;
    std::vector<std::string *> *dataRow;;

    std::vector<std::vector<std::string *> *>::iterator itr;
    for (itr = dataStorage.begin(); itr < dataStorage.end(); itr++) {
        if (firstDone) {
            ss << "), (";
        } else {
            firstDone = true;
        }

        dataRow = *itr;

        if (columns != dataRow->size()) {
            throw new std::invalid_argument("Incorrect amount of data supplied.");
        }

        for (uint32_t column = 0; column < columns; column++) {
            ss << dataRow->at(column);

            if (column < columns-1) {
                ss << ", ";
            }

            delete dataRow->at(column);
            dataRow->at(column) = 0;
        }

        delete dataRow;
    }

    dataStorage.clear();

    ss << ");";

    setQuery(ss.str());
    return Query::execute();
}
