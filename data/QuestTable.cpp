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

#include "data/QuestTable.hpp"

#include <iostream>

#include <boost/shared_ptr.hpp>

#include "db/SelectQuery.hpp"
#include "db/Result.hpp"

#include "Logger.hpp"

QuestTable::QuestTable(): _dataOK(false) {
    reload();
}

void QuestTable::reload() {
#ifdef DataConnect_DEBUG
    std::cout<<"QuestTable: reload!" <<std::endl;
#endif

    try {
        Database::SelectQuery query;
        query.addColumn("quests", "qst_id");
        query.addColumn("quests", "qst_script");
        query.addServerTable("quests");

        Database::Result results = query.execute();

        if (!results.empty()) {
            quests.clear();
            std::string scriptname;

            for (Database::ResultConstIterator itr = results.begin(); itr != results.end(); ++itr) {
                if (!((*itr)["qst_script"].is_null())) {
                    scriptname = ((*itr)["qst_script"].as<std::string>());

                    try {
                        boost::shared_ptr<LuaQuestScript> script(new LuaQuestScript(scriptname));
                        TYPE_OF_QUEST_ID id = ((*itr)["qst_id"].as<TYPE_OF_QUEST_ID>());
                        quests[id] = script;
                    } catch (ScriptException &e) {
                        Logger::writeError("scripts", "Error while loading quest script: " + scriptname + ":\n" + e.what() + "\n");
                    }
                }
            }
        }

        std::cout << "loaded quest scripts! " << std::endl;
        _dataOK = true;
    } catch (std::exception &e) {
        std::cerr << "exception: " << e.what() << std::endl;
        _dataOK = false;
    }
}

boost::shared_ptr<LuaQuestScript> QuestTable::getQuestScript(TYPE_OF_QUEST_ID id) {
    auto iterator = quests.find(id);

    if (iterator == quests.end()) {
        boost::shared_ptr<LuaQuestScript> notFound;
        return notFound;
    } else {
        return iterator->second;
    }
}

