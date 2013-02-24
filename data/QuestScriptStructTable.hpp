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

#ifndef _QUEST_SCRIPT_STRUCT_TABLE_HPP_
#define _QUEST_SCRIPT_STRUCT_TABLE_HPP_

#include "data/ScriptStructTable.hpp"
#include "data/QuestNodeTable.hpp"
#include "Logger.hpp"

template<typename IdType, typename StructType, typename ScriptType, typename ScriptParameter = StructType>
class QuestScriptStructTable : public ScriptStructTable<IdType, StructType, ScriptType, ScriptParameter> {
public:
    typedef ScriptStructTable<IdType, StructType, ScriptType, ScriptParameter> Base;

    virtual void reloadScripts() {
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
                Logger::error(LogFacility::Script) << "Error while loading " << getTableName() << " quest script: "
                                   << questNode.script->getFileName() << ": " << e.what() << Log::end;
            }
        }
    }

protected:
    typedef QuestNodeTable::TableRange<IdType> NodeRange;

    virtual std::string getTableName() = 0;
    virtual std::vector<std::string> getColumnNames() = 0;
    virtual IdType assignId(const Database::ResultTuple &row) = 0;
    virtual StructType assignTable(const Database::ResultTuple &row) = 0;
    virtual std::string assignScriptName(const Database::ResultTuple &row) = 0;
    virtual NodeRange getQuestScripts() = 0;
};

#endif

