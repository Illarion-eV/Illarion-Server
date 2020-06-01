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

#ifndef STRUCT_TABLE_HPP
#define STRUCT_TABLE_HPP

#include "Logger.hpp"
#include "data/Table.hpp"
#include "db/Result.hpp"
#include "db/SelectQuery.hpp"

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

template <typename IdType, typename StructType> class StructTable : public Table {
    using ContainerType = std::unordered_map<IdType, StructType>;

public:
    auto reloadBuffer() -> bool override {
        try {
            Database::SelectQuery query;

            for (const auto &column : getColumnNames()) {
                query.addColumn(column);
            }

            query.setServerTable(getTableName());
            const auto results = query.execute();

            clear();

            for (const auto &row : results) {
                evaluateRow(row);
            }

            isBufferValid = true;
        } catch (std::exception &e) {
            Logger::warn(LogFacility::Database)
                    << "Exception in loading table " << getTableName() << ": " << e.what() << Log::end;
            isBufferValid = false;
        }

        return isBufferValid;
    }

    void reloadScripts() override {}

    void activateBuffer() override {
        structs.swap(structBuffer);
        isBufferValid = false;
        clear();
    }

    auto exists(const IdType &id) const -> bool {
        return structs.count(id) > 0;
    }

    auto operator[](const IdType &id) -> const StructType & {
        try {
            return structs.at(id);
        } catch (std::out_of_range &) {
            Logger::error(LogFacility::Script)
                    << "Table " << getTableName() << ": entry " << id << " was not found!" << Log::end;
            return structs[id];
        }
    }

    auto get(const IdType &id) const -> const StructType & {
        return structs.at(id);
    }

    auto begin() const -> typename ContainerType::const_iterator {
        return structs.cbegin();
    }

    auto end() const -> typename ContainerType::const_iterator {
        return structs.cend();
    }

protected:
    virtual auto getTableName() -> std::string = 0;
    virtual auto getColumnNames() -> std::vector<std::string> = 0;
    virtual auto assignId(const Database::ResultTuple &row) -> IdType = 0;
    virtual auto assignTable(const Database::ResultTuple &row) -> StructType = 0;

    virtual void clear() {
        structBuffer.clear();
    }

    virtual void evaluateRow(const Database::ResultTuple &row) {
        emplace(assignId(row), assignTable(row));
    }

    virtual void emplace(const IdType &id, const StructType &data) {
        structBuffer.emplace(id, data);
    }

    auto erase(const IdType &id) -> bool {
        return structs.erase(id) > 0;
    }

    auto get(const IdType &id) -> StructType & {
        return structs[id];
    }

private:
    ContainerType structs;
    ContainerType structBuffer;
    bool isBufferValid = false;
};

#endif
