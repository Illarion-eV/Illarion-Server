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

#include "data/CommonObjectTable.hpp"

#include <iostream>

#include "db/SelectQuery.hpp"
#include "db/Result.hpp"

#include "script/LuaItemScript.hpp"

#include "Logger.hpp"
#include "World.hpp"
#include "data/QuestNodeTable.hpp"

CommonObjectTable::CommonObjectTable() : m_dataOK(false) {
    reload();
}

TYPE_OF_ITEM_ID CommonObjectTable::calcInfiniteRot(TYPE_OF_ITEM_ID id, std::map<TYPE_OF_ITEM_ID, bool> &visited, std::map<TYPE_OF_ITEM_ID, bool> &assigned) {
    if (visited[ id ]) {
        if (assigned[ id ]) {
            return m_table[ id ].AfterInfiniteRot;
        }

        return 0;
    }

    visited[ id ] = true;

    if (id == m_table[ id ].ObjectAfterRot) {
        if (m_table[ id ].AgeingSpeed >= 255) {
            m_table[ id ].AfterInfiniteRot = id;
            assigned[ id ] = true;
            return id;
        }

        m_table[ id ].AfterInfiniteRot = 0;
        assigned[ id ] = true;
        return 0;
    }

    m_table[ id ].AfterInfiniteRot = calcInfiniteRot(m_table[ id ].ObjectAfterRot, visited, assigned);
    assigned[ id ] = true;
    return m_table[ id ].AfterInfiniteRot;
}

void CommonObjectTable::reload() {

#ifdef DataConnect_DEBUG
    std::cout << "CommonObjectTable: reload" << std::endl;
#endif

    try {
        Database::SelectQuery query;
        query.addColumn("common", "com_itemid");
        query.addColumn("common", "com_weight");
        query.addColumn("common", "com_agingspeed");
        query.addColumn("common", "com_objectafterrot");
        query.addColumn("common", "com_rotsininventory");
        query.addColumn("common", "com_script");
        query.addColumn("common", "com_brightness");
        query.addColumn("common", "com_worth");
        query.addColumn("common", "com_buystack");
        query.addColumn("common", "com_maxstack");
        query.addServerTable("common");
        query.addOrderBy("common", "com_itemid", Database::SelectQuery::ASC);

        Database::Result results = query.execute();

        std::map<TYPE_OF_ITEM_ID, bool> assigned; // map item id to whether an infinite rot has been assigned
        std::map<TYPE_OF_ITEM_ID, bool> visited;  // map item id to whether it has been visited already in calcInfiniteRot

        if (!results.empty()) {
            clearOldTable();
            CommonStruct temprecord;
            auto questNodes = QuestNodeTable::getInstance()->getItemNodes();
            auto questItr = questNodes.first;
            auto questEnd = questNodes.second;

            for (Database::ResultConstIterator itr = results.begin();
                 itr != results.end(); ++itr) {

                TYPE_OF_ITEM_ID itemID;
                itemID = (TYPE_OF_ITEM_ID)((*itr)["com_itemid"].as<int16_t>());
                temprecord.id = itemID;
                temprecord.Weight = (TYPE_OF_WEIGHT)((*itr)["com_weight"].as<int16_t>());
                temprecord.AgeingSpeed = (TYPE_OF_AGINGSPEED)((*itr)["com_agingspeed"].as<int16_t>());
                temprecord.ObjectAfterRot = (TYPE_OF_ITEM_ID)(*itr)["com_objectafterrot"].as<TYPE_OF_ITEM_ID>();
                temprecord.rotsInInventory = (*itr)["com_rotsininventory"].as<bool>();
                temprecord.Brightness = (TYPE_OF_BRIGHTNESS)((*itr)["com_brightness"].as<int16_t>());
                temprecord.Worth = (TYPE_OF_WORTH)((*itr)["com_worth"].as<TYPE_OF_WORTH>());
                temprecord.BuyStack = (TYPE_OF_BUY_STACK)((*itr)["com_buystack"].as<TYPE_OF_BUY_STACK>());
                temprecord.MaxStack = (TYPE_OF_MAX_STACK)((*itr)["com_maxstack"].as<TYPE_OF_MAX_STACK>());

                if (!((*itr)["com_script"].is_null())) {
                    std::string scriptname = ((*itr)["com_script"].as<std::string>());

                    try {
                        std::shared_ptr<LuaItemScript> tmpScript(new LuaItemScript(scriptname, temprecord));
                        m_scripttable[itemID] = tmpScript;
                    } catch (ScriptException &e) {
                        Logger::error(LogFacility::Script) << "Error while loading item script: " << scriptname << ": " << e.what() << Log::end;
                    }
                } else if (questItr != questEnd && questItr->first == itemID) {
                    std::shared_ptr<LuaItemScript> tmpScript(new LuaItemScript(temprecord));
                    m_scripttable[itemID] = tmpScript;
                }

                try {
                    while (questItr != questEnd && questItr->first == itemID) {
                        m_scripttable[itemID]->addQuestScript(questItr->second.entrypoint, questItr->second.script);
                        ++questItr;
                    }
                } catch (ScriptException &e) {
                    Logger::error(LogFacility::Script) << "Error while loading item quest script: " << questItr->second.script->getFileName() << ": " << e.what() << Log::end;
                }


                m_table[itemID] = temprecord;

                visited.insert(std::pair<TYPE_OF_ITEM_ID, bool>(itemID,false));
                assigned.insert(std::pair<TYPE_OF_ITEM_ID, bool>(itemID,false));
            }

            m_dataOK = true;
        } else {
            m_dataOK = false;
        }


#ifdef DataConnect_DEBUG
        std::cout << "loaded " << rows << " rows into CommonObjectTable" << std::endl;
#endif

    } catch (std::exception &e) {
        std::cerr << "exception: " << e.what() << std::endl;
        m_dataOK = false;
    }

}

const CommonStruct &CommonObjectTable::find(TYPE_OF_ITEM_ID id) {
    try {
        return m_table.at(id);
    } catch (std::out_of_range &) {
        Logger::error(LogFacility::Script) << "CommonObjectTable: item " << id << " was not found!" << Log::end;
        return m_table[id];
    }
}

std::shared_ptr<LuaItemScript> CommonObjectTable::findScript(TYPE_OF_ITEM_ID Id) {
    SCRIPTTABLE::iterator iterator;
    iterator = m_scripttable.find(Id);

    if (iterator != m_scripttable.end()) {
        return iterator->second;
    } else {
        return std::shared_ptr<LuaItemScript>();
    }
}

void CommonObjectTable::clearOldTable() {
    m_table.clear();
}

CommonObjectTable::~CommonObjectTable() {
    clearOldTable();
}

