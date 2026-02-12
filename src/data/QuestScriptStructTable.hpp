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

#ifndef QUEST_SCRIPT_STRUCT_TABLE_HPP
#define QUEST_SCRIPT_STRUCT_TABLE_HPP

#include "Logger.hpp"
#include "data/QuestNodeTable.hpp"
#include "data/ScriptStructTable.hpp"

/**
 * @brief Extension of ScriptStructTable that adds quest script support
 *
 * Extends ScriptStructTable to support quest-related scripts in addition to
 * the main item/NPC scripts. Quest scripts are loaded from the QuestNodeTable
 * and added to the script stack using addQuestScript().
 *
 * This allows items, NPCs, etc. to have both:
 * - A primary script (from the main table)
 * - Additional quest scripts (from quest nodes)
 *
 * Quest scripts are typically used to add quest-specific behavior without
 * modifying the main script.
 *
 * @tparam IdType Type of the unique identifier
 * @tparam StructType Type of the data structure stored
 * @tparam ScriptType Type of Lua script that supports quest scripts
 * @tparam ScriptParameter Type passed to script constructor (defaults to StructType)
 */
template <typename IdType, typename StructType, typename ScriptType, typename ScriptParameter = StructType>
class QuestScriptStructTable : public ScriptStructTable<IdType, StructType, ScriptType, ScriptParameter> {
public:
    using Base = ScriptStructTable<IdType, StructType, ScriptType, ScriptParameter>;

    /**
     * @brief Load both main scripts and quest scripts
     *
     * First calls base class to load main scripts, then adds quest scripts
     * from the quest node table. Creates script objects if they don't exist
     * yet (for items/NPCs that only have quest scripts).
     */
    void reloadScripts() override {
        Base::reloadScripts();
        auto questNodes = getQuestScripts();

        for (auto it = questNodes.first; it != questNodes.second; ++it) {
            const auto &id = it->first;
            const auto &questNode = it->second;
            auto &scriptStack = this->scriptNonConst(id);

            if (!scriptStack) {
                scriptStack = std::make_shared<ScriptType>();
            }

            try {
                scriptStack->addQuestScript(questNode.entrypoint, questNode.script);
            } catch (ScriptException &e) {
                Logger::error(LogFacility::Script)
                        << "Error while loading " << getTableName()
                        << " quest script: " << questNode.script->getFileName() << ": " << e.what() << Log::end;
            }
        }
    }

protected:
    using NodeRange = QuestNodeTable::TableRange<IdType>;

    using Base::assignId;
    using Base::assignScriptName;
    using Base::assignTable;
    using Base::getColumnNames;
    using Base::getTableName;

    /**
     * @brief Get quest script nodes for this table's entries
     * @return Iterator range over quest nodes relevant to this table
     */
    virtual auto getQuestScripts() -> NodeRange = 0;
};

#endif
