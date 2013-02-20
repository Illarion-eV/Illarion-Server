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

#ifndef _SCRIPT_STRUCT_TABLE_HPP_
#define _SCRIPT_STRUCT_TABLE_HPP_

#include <string>
#include <vector>
#include <unordered_map>
#include "data/StructTable.hpp"
#include "Logger.hpp"
#include "script/LuaScript.hpp"

namespace detail {

template<typename IdType, typename StructType, typename ScriptType>
void detailAssign(std::unordered_map<IdType, std::shared_ptr<ScriptType>> &scripts, const IdType &id, const std::string &name, const StructType &data, const StructType &dummy) {
    scripts.emplace(id, std::make_shared<ScriptType>(name, data));
}

template<typename IdType, typename StructType, typename ScriptType>
void detailAssign(std::unordered_map<IdType, std::shared_ptr<ScriptType>> &scripts, const IdType &id, const std::string &name, const StructType &data, const IdType &dummy) {
    scripts.emplace(id, std::make_shared<ScriptType>(name, id));
}

}

template<typename IdType, typename StructType, typename ScriptType, typename ScriptParameter = StructType>
class ScriptStructTable : public StructTable<IdType, StructType> {
public:
    typedef StructTable<IdType, StructType> Base;

    virtual void reloadScripts() {
        scripts.clear();

        for (const auto &scriptNameEntry : scriptNames) {
            const IdType &id = scriptNameEntry.first;
            const std::string &scriptName = scriptNameEntry.second;

            try {
                const auto &data = (*this)[id];
                internalAssign<ScriptParameter>(id, scriptName, data);
            } catch (ScriptException &e) {
                Logger::writeError("scripts", "Error while loading " + getTableName() + " script: " + scriptName + ":\n" + e.what() + "\n");
            }
        }

        scriptNames.clear();
    }

    const std::shared_ptr<ScriptType> &script(IdType id) {
        return scripts[id];
    }

protected:
    virtual std::string getTableName() = 0;
    virtual std::vector<std::string> getColumnNames() = 0;
    virtual IdType assignId(const Database::ResultTuple &row) = 0;
    virtual StructType assignTable(const Database::ResultTuple &row) = 0;
    virtual std::string assignScriptName(const Database::ResultTuple &row) = 0;

    virtual void evaluateRow(const Database::ResultTuple &row) {
        Base::evaluateRow(row);
        std::string scriptName = assignScriptName(row);

        if (!scriptName.empty()) {
            scriptNames.emplace_back(assignId(row), scriptName);
        }
    }

    std::shared_ptr<ScriptType> &scriptNonConst(IdType id) {
        return scripts[id];
    }

private:
    typedef std::unordered_map<IdType, std::shared_ptr<ScriptType>> ScriptsType;

    template<typename T>
    void internalAssign(const IdType &id, const std::string &name, const StructType &data);

    typedef std::vector<std::pair<IdType, std::string>> NamesType;
    NamesType scriptNames;
    ScriptsType scripts;
};

template<typename IdType, typename StructType, typename ScriptType, typename ScriptParameter>
template<typename T>
void ScriptStructTable<IdType, StructType, ScriptType, ScriptParameter>::internalAssign(const IdType &id, const std::string &name, const StructType &data) {
    detail::detailAssign<IdType, StructType, ScriptType>(scripts, id, name, data, ScriptParameter());
}

#endif

