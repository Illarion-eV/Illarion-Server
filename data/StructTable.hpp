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

#include <vector>
#include <string>
#include <unordered_map>
#include "data/Table.hpp"
#include "db/Result.hpp"
#include "db/SelectQuery.hpp"
#include "script/LuaScript.hpp"
#include "Logger.hpp"
#include <boost/make_shared.hpp>

template<typename IdType, typename StructType, typename ScriptType>
class StructTable : public Table {
public:
    virtual bool reloadBuffer() {
        try {
            Database::SelectQuery query;

for (const auto &column : getColumnNames()) {
                query.addColumn(column);
            }

            query.setServerTable(getTableName());
            Database::Result results = query.execute();

            structBuffer.clear();
            scriptNames.clear();

for (const auto &row : results) {
                IdType id = assignId(row);
                structBuffer.emplace(id, assignTable(row));

                std::string scriptName = assignScriptName(row);

                if (!scriptName.empty()) {
                    scriptNames.emplace_back(id, scriptName);
                }
            }

            isBufferValid = true;
        } catch (std::exception &e) {
            std::cerr << "Exception in loading table " << getTableName() << ": " << e.what() << std::endl;
            isBufferValid = false;
        }

        return isBufferValid;
    }

    virtual void reloadScripts() {
for (const auto &scriptNameEntry : scriptNames) {
            const IdType &id = scriptNameEntry.first;
            const std::string &scriptName = scriptNameEntry.second;

            try {
                auto script = boost::make_shared<ScriptType>(scriptName, structBuffer[id]);
                structBuffer[id].script = script;
            } catch (ScriptException &e) {
                Logger::writeError("scripts", "Error while loading " + getTableName() + " script: " + scriptName + ":\n" + e.what() + "\n");
            }
        }

        scriptNames.clear();
    }

    virtual void activateBuffer() {
        structs.swap(structBuffer);
        isBufferValid = false;
        structBuffer.clear();
        scriptNames.clear();
    }

    const StructType &find(IdType id) {
        try {
            return structs.at(id);
        } catch (std::out_of_range &) {
            std::stringstream ss;
            ss << "Table " << getTableName() << ": entry " << id << " was not found!\n";
            Logger::writeError("scripts", ss.str());
            return structs[id];
        }
    }

protected:
    virtual std::string getTableName() = 0;
    virtual std::vector<std::string> getColumnNames() = 0;
    virtual IdType assignId(const Database::ResultTuple &row) = 0;
    virtual StructType assignTable(const Database::ResultTuple &row) = 0;
    virtual std::string assignScriptName(const Database::ResultTuple &row) = 0;

private:
    typedef std::unordered_map<IdType, StructType> ContainerType;
    typedef std::vector<std::pair<IdType, std::string>> NamesType;

    ContainerType structs;
    ContainerType structBuffer;
    NamesType scriptNames;
    bool isBufferValid = false;
};

#endif

