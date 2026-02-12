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

#include "Logger.hpp"
#include "data/StructTable.hpp"
#include "script/LuaScript.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace detail {

/**
 * @brief Helper function to create scripts with StructType parameter
 * @tparam IdType ID type
 * @tparam StructType Data structure type
 * @tparam ScriptType Script type
 * @param scripts Map to store created script
 * @param id Entry ID
 * @param name Script filename
 * @param data Data structure for script
 * @param dummy Type discriminator (StructType)
 */
template <typename IdType, typename StructType, typename ScriptType>
void detailAssign(std::unordered_map<IdType, std::shared_ptr<ScriptType>> &scripts, const IdType &id,
                  const std::string &name, const StructType &data, const StructType &dummy) {
    scripts.emplace(id, std::make_shared<ScriptType>(name, data));
}

/**
 * @brief Helper function to create scripts with IdType parameter
 * @tparam IdType ID type
 * @tparam StructType Data structure type
 * @tparam ScriptType Script type
 * @param scripts Map to store created script
 * @param id Entry ID
 * @param name Script filename
 * @param data Data structure (unused in this overload)
 * @param dummy Type discriminator (IdType)
 */
template <typename IdType, typename StructType, typename ScriptType>
void detailAssign(std::unordered_map<IdType, std::shared_ptr<ScriptType>> &scripts, const IdType &id,
                  const std::string &name, const StructType &data, const IdType &dummy) {
    scripts.emplace(id, std::make_shared<ScriptType>(name, id));
}

} // namespace detail

/**
 * @brief Template table class for data with associated Lua scripts
 *
 * Extends StructTable to support loading and managing Lua scripts associated
 * with table entries. Each entry can have an optional script that implements
 * custom behavior (e.g., item use scripts, NPC scripts).
 *
 * Scripts are loaded during reloadScripts() by reading script filenames from
 * the database and instantiating the appropriate script objects. The
 * ScriptParameter template parameter determines what gets passed to the
 * script constructor - either the full data structure or just the ID.
 *
 * @tparam IdType Type of the unique identifier
 * @tparam StructType Type of the data structure stored
 * @tparam ScriptType Type of Lua script (e.g., LuaItemScript, LuaNPCScript)
 * @tparam ScriptParameter Type passed to script constructor (defaults to StructType)
 */
template <typename IdType, typename StructType, typename ScriptType, typename ScriptParameter = StructType>
class ScriptStructTable : public StructTable<IdType, StructType> {
public:
    using Base = StructTable<IdType, StructType>;

    /**
     * @brief Load and compile Lua scripts for table entries
     *
     * Instantiates script objects for all entries that have script files
     * defined. Clears any previously loaded scripts first.
     */
    void reloadScripts() override {
        scripts.clear();

        for (const auto &scriptNameEntry : scriptNames) {
            const IdType &id = scriptNameEntry.first;
            const std::string &scriptName = scriptNameEntry.second;

            try {
                const auto &data = (*this)[id];
                internalAssign<ScriptParameter>(id, scriptName, data);
            } catch (ScriptException &e) {
                Logger::error(LogFacility::Script) << "Error while loading " << getTableName()
                                                   << " script: " << scriptName << ": " << e.what() << Log::end;
            }
        }

        scriptNames.clear();
    }

    /**
     * @brief Get the script for a specific entry
     * @param id Entry ID
     * @return Shared pointer to script, or nullptr if no script exists
     */
    auto script(IdType id) const -> std::shared_ptr<ScriptType> {
        auto it = scripts.find(id);

        if (it != scripts.end()) {
            return it->second;
        }
        return {};
    }

protected:
    using Base::assignId;
    using Base::assignTable;
    using Base::getColumnNames;
    using Base::getTableName;

    /**
     * @brief Extract script filename from database row
     * @param row Database result row
     * @return Script filename, or empty string if no script
     */
    virtual auto assignScriptName(const Database::ResultTuple &row) -> std::string = 0;

    /**
     * @brief Process database row, extracting both data and script name
     * @param row Database result row
     */
    void evaluateRow(const Database::ResultTuple &row) override {
        Base::evaluateRow(row);
        std::string scriptName = assignScriptName(row);

        if (!scriptName.empty()) {
            scriptNames.emplace_back(assignId(row), scriptName);
        }
    }

    /**
     * @brief Get mutable reference to script (for derived classes)
     * @param id Entry ID
     * @return Mutable reference to script shared pointer
     */
    auto scriptNonConst(IdType id) -> std::shared_ptr<ScriptType> & { return scripts[id]; }

private:
    using ScriptsType = std::unordered_map<IdType, std::shared_ptr<ScriptType>>;

    template <typename T> void internalAssign(const IdType &id, const std::string &name, const StructType &data);

    using NamesType = std::vector<std::pair<IdType, std::string>>;
    NamesType scriptNames;  ///< Script filenames collected during buffer load
    ScriptsType scripts;    ///< Active script instances indexed by ID
};

/**
 * @brief Internal helper to create script with appropriate parameters
 * @tparam T Type discriminator for overload resolution
 * @param id Entry ID
 * @param name Script filename
 * @param data Data structure
 */
template <typename IdType, typename StructType, typename ScriptType, typename ScriptParameter>
template <typename T>
void ScriptStructTable<IdType, StructType, ScriptType, ScriptParameter>::internalAssign(const IdType &id,
                                                                                        const std::string &name,
                                                                                        const StructType &data) {
    detail::detailAssign<IdType, StructType, ScriptType>(scripts, id, name, data, ScriptParameter());
}

#endif
