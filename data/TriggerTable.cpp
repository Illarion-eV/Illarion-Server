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

#include "data/TriggerTable.hpp"

#include <iostream>

#include <boost/cstdint.hpp>
#include <memory>

#include "db/SelectQuery.hpp"
#include "db/Result.hpp"

#include "data/QuestNodeTable.hpp"

#include "Logger.hpp"

template<class from>
const std::string toString(const from &convert) {
    std::stringstream stream;
    stream << convert;
    return stream.str();
}


TriggerTable::TriggerTable():  _dataOK(false) {
    reload();
}

void TriggerTable::reload() {
#ifdef DataConnect_DEBUG
    std::cout<<"TriggerTable: reload!" <<std::endl;
#endif

    try {
        Database::SelectQuery query;
        query.addColumn("triggerfields", "tgf_posx");
        query.addColumn("triggerfields", "tgf_posy");
        query.addColumn("triggerfields", "tgf_posz");
        query.addColumn("triggerfields", "tgf_script");
        query.addServerTable("triggerfields");

        Database::Result results = query.execute();

        if (!results.empty()) {
            clearOldTable();
            TriggerStruct Trigger;
            std::string scriptname;

            for (Database::ResultConstIterator itr = results.begin();
                 itr != results.end(); ++itr) {
                Trigger.pos = position(((*itr)["tgf_posx"].as<int32_t>()),
                                       ((*itr)["tgf_posy"].as<int32_t>()),
                                       ((*itr)["tgf_posz"].as<int32_t>()));

                if (!((*itr)["tgf_script"].is_null())) {
                    scriptname = ((*itr)["tgf_script"].as<std::string>());

                    try {
                        std::shared_ptr<LuaTriggerScript> script(new LuaTriggerScript(scriptname, Trigger.pos));
                        Trigger.script = script;
                    } catch (ScriptException &e) {
                        Logger::writeError("scripts", "Error while loading trigger script: " + scriptname + ":\n" + e.what() + "\n");
                    }
                }

                Triggers.insert(std::pair<position, TriggerStruct>(Trigger.pos, Trigger));
            }
        }

        auto questNodes = QuestNodeTable::getInstance()->getTriggerNodes();
        TriggerStruct trigger;

        for (auto questItr = questNodes.first; questItr != questNodes.second; ++questItr) {
            if (find(questItr->pos, trigger)) {
                trigger.script->addQuestScript(questItr->entrypoint, questItr->script);
            } else {
                trigger.pos = questItr->pos;
                std::shared_ptr<LuaTriggerScript> script(new LuaTriggerScript(trigger.pos));
                trigger.scriptname = "";
                trigger.script = script;
                trigger.script->addQuestScript(questItr->entrypoint, questItr->script);
                Triggers.insert(std::pair<position, TriggerStruct>(trigger.pos, trigger));
            }
        }

        std::cout << " loaded trigger fields! " << std::endl;
        _dataOK = true;
    } catch (std::exception &e) {

        std::cerr << "exception: " << e.what() << std::endl;
        _dataOK = false;
    }
}

bool TriggerTable::find(position pos, TriggerStruct &data) {
    TriggerMap::iterator iterator;
    iterator = Triggers.find(pos);

    if (iterator == Triggers.end()) {
        return false;
    } else {
        data = (*iterator).second;
        return true;
    }

}

void TriggerTable::clearOldTable() {
    Triggers.clear();
}

TriggerTable::~TriggerTable() {
    clearOldTable();
}

