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

#ifndef SCRIPT_STRUCT_TABLE_HPP
#define SCRIPT_STRUCT_TABLE_HPP

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
    using Base = StructTable<IdType, StructType>;

    void reloadScripts() override {
        scripts.clear();

        for (const auto &scriptNameEntry : scriptNames) {
            const IdType &id = scriptNameEntry.first;
            const std::string &scriptName = scriptNameEntry.second;

            try {
                const auto &data = (*this)[id];
                internalAssign<ScriptParameter>(id, scriptName, data);
            } catch (ScriptException &e) {
                Logger::error(LogFacility::Script) << "Error while loading " << getTableName() << " script: " << scriptName << ": " << e.what() << Log::end;
            }
        }

        scriptNames.clear();
    }

    auto script(IdType id) const -> const std::shared_ptr<ScriptType> {
        auto it = scripts.find(id);

        if (it != scripts.end()) {
            return it->second;
        } else {
            return {};
        }
    }

protected:
    using Base::getTableName;
    using Base::getColumnNames;
    using Base::assignId;
    using Base::assignTable;
    virtual auto assignScriptName(const Database::ResultTuple &row) -> std::string = 0;

    void evaluateRow(const Database::ResultTuple &row) override {
        Base::evaluateRow(row);
        std::string scriptName = assignScriptName(row);

        if (!scriptName.empty()) {
            scriptNames.emplace_back(assignId(row), scriptName);
        }
    }

    auto scriptNonConst(IdType id) -> std::shared_ptr<ScriptType> & {
        return scripts[id];
    }

private:
    using ScriptsType = std::unordered_map<IdType, std::shared_ptr<ScriptType>>;

    template<typename T>
    void internalAssign(const IdType &id, const std::string &name, const StructType &data);

    using NamesType = std::vector<std::pair<IdType, std::string>>;
    NamesType scriptNames;
    ScriptsType scripts;
};

template<typename IdType, typename StructType, typename ScriptType, typename ScriptParameter>
template<typename T>
void ScriptStructTable<IdType, StructType, ScriptType, ScriptParameter>::internalAssign(const IdType &id, const std::string &name, const StructType &data) {
    detail::detailAssign<IdType, StructType, ScriptType>(scripts, id, name, data, ScriptParameter());
}

#endif

