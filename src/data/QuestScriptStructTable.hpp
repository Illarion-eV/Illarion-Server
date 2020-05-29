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

#ifndef _QUEST_SCRIPT_STRUCT_TABLE_HPP_
#define _QUEST_SCRIPT_STRUCT_TABLE_HPP_

#include "data/ScriptStructTable.hpp"
#include "data/QuestNodeTable.hpp"
#include "Logger.hpp"

template<typename IdType, typename StructType, typename ScriptType, typename ScriptParameter = StructType>
class QuestScriptStructTable : public ScriptStructTable<IdType, StructType, ScriptType, ScriptParameter> {
public:
    using Base = ScriptStructTable<IdType, StructType, ScriptType, ScriptParameter>;

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
                Logger::error(LogFacility::Script) << "Error while loading " << getTableName() << " quest script: "
                                                   << questNode.script->getFileName() << ": " << e.what() << Log::end;
            }
        }
    }

protected:
    using NodeRange = QuestNodeTable::TableRange<IdType>;

    using Base::getTableName;
    using Base::getColumnNames;
    using Base::assignId;
    using Base::assignTable;
    using Base::assignScriptName;

    virtual NodeRange getQuestScripts() = 0;
};

#endif

