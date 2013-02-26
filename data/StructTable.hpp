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

#ifndef _STRUCT_TABLE_HPP_
#define _STRUCT_TABLE_HPP_

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include "data/Table.hpp"
#include "db/Result.hpp"
#include "db/SelectQuery.hpp"
#include "Logger.hpp"

template<typename IdType, typename StructType>
class StructTable : public Table {
    typedef std::unordered_map<IdType, StructType> ContainerType;
public:
    virtual bool reloadBuffer() {
        try {
            Database::SelectQuery query;

for (const auto &column : getColumnNames()) {
                query.addColumn(column);
            }

            query.setServerTable(getTableName());
            Database::Result results = query.execute();

            clear();

for (const auto &row : results) {
                evaluateRow(row);
            }

            isBufferValid = true;
        } catch (std::exception &e) {
            Logger::warn(LogFacility::Database) << "Exception in loading table " << getTableName() << ": " << e.what() << Log::end;
            isBufferValid = false;
        }

        return isBufferValid;
    }

    virtual void reloadScripts() {}

    virtual void activateBuffer() {
        structs.swap(structBuffer);
        isBufferValid = false;
        clear();
    }

    bool exists(const IdType &id) const {
        return structs.count(id) > 0;
    }

    const StructType &operator[](const IdType &id) {
        try {
            return structs.at(id);
        } catch (std::out_of_range &) {
            std::stringstream ss;
            Logger::error(LogFacility::Script) << "Table " << getTableName() << ": entry " << id << " was not found!" << Log::end;
            return structs[id];
        }
    }

    typename ContainerType::const_iterator begin() const {
        return structs.cbegin();
    }

    typename ContainerType::const_iterator end() const {
        return structs.cend();
    }

protected:
    virtual std::string getTableName() = 0;
    virtual std::vector<std::string> getColumnNames() = 0;
    virtual IdType assignId(const Database::ResultTuple &row) = 0;
    virtual StructType assignTable(const Database::ResultTuple &row) = 0;

    virtual void clear() {
        structBuffer.clear();
    }

    virtual void evaluateRow(const Database::ResultTuple &row) {
        structBuffer.emplace(assignId(row), assignTable(row));
    }

    bool erase(const IdType &id) {
        return structs.erase(id) > 0;
    }

    StructType &get(const IdType &id) {
        return structs[id];
    }

private:
    ContainerType structs;
    ContainerType structBuffer;
    bool isBufferValid = false;
};

#endif

