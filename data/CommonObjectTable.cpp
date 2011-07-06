//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.


#include "CommonObjectTable.hpp"

#include <iostream>
#include <boost/shared_ptr.hpp>

#include "db/ConnectionManager.hpp"
#include "db/SchemaHelper.hpp"
#include "db/SelectQuery.hpp"
#include "db/Result.hpp"

#include "script/LuaItemScript.hpp"

#include "Logger.hpp"
#include "World.hpp"

CommonObjectTable::CommonObjectTable() : m_dataOK(false) {
    reload();
}

TYPE_OF_ITEM_ID CommonObjectTable::calcInfiniteRot(TYPE_OF_ITEM_ID id, std::map<TYPE_OF_ITEM_ID, bool> & visited, std::map<TYPE_OF_ITEM_ID, bool> & assigned) {
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
        Database::PConnection connection =
            Database::ConnectionManager::getInstance().getConnection();

        Database::SelectQuery query(*connection);
        query.addColumn("common", "com_itemid");
        query.addColumn("common", "com_volume");
        query.addColumn("common", "com_weight");
        query.addColumn("common", "com_agingspeed");
        query.addColumn("common", "com_objectafterrot");
        query.addColumn("common", "com_script");
        query.addColumn("common", "com_brightness");
        query.addColumn("common", "com_worth");
        query.addServerTable("common");

        Database::Result results = query.execute();
        Database::ConnectionManager::getInstance().releaseConnection(connection);

        std::map<TYPE_OF_ITEM_ID, bool> assigned; // map item id to whether an infinite rot has been assigned
        std::map<TYPE_OF_ITEM_ID, bool> visited;  // map item id to whether it has been visited already in calcInfiniteRot

        if (!results.empty()) {
            clearOldTable();
            CommonStruct temprecord;

            for (Database::Result::ConstIterator itr = results.begin();
                 itr != results.end(); ++itr) {

                TYPE_OF_ITEM_ID itemID;
                itemID = (TYPE_OF_ITEM_ID)((*itr)["com_itemid"].as<int16_t>());
                temprecord.id = itemID;
                temprecord.Weight = (TYPE_OF_WEIGHT)((*itr)["com_weight"].as<int16_t>());
                temprecord.Volume = (TYPE_OF_VOLUME)((*itr)["com_volume"].as<int16_t>());
                temprecord.AgeingSpeed = (TYPE_OF_AGEINGSPEED)((*itr)["com_agingspeed"].as<int16_t>());
                temprecord.ObjectAfterRot = (TYPE_OF_ITEM_ID)(*itr)["com_objectafterrot"].as<TYPE_OF_ITEM_ID>();
                temprecord.isStackable = (*itr)["com_stackable"].as<bool>();
                temprecord.rotsInInventory = (*itr)["com_rotsininventory"].as<bool>();
                temprecord.Brightness = (TYPE_OF_BRIGHTNESS)((*itr)["com_brightness"].as<int16_t>());
                temprecord.Worth = (TYPE_OF_WORTH)((*itr)["com_worth"].as<int16_t>());

                if (!((*itr)["com_worth"].is_null())) {
                    std::string scriptname = ((*itr)["com_worth"].as<std::string>());

                    try {
                        boost::shared_ptr<LuaItemScript> tmpScript(new LuaItemScript(scriptname, temprecord));
                        m_scripttable[itemID] = tmpScript;
                    } catch (ScriptException &e) {
                        Logger::writeError("scripts", "Error while loading script: " + scriptname + ":\n" + e.what() + "\n");
                    }
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

bool CommonObjectTable::find(TYPE_OF_ITEM_ID Id, CommonStruct &ret) {
    TABLE::iterator iterator;
    iterator = m_table.find(Id);

    if (iterator == m_table.end()) {
        return false;
    } else {
        ret = (*iterator).second;
        return true;
    }
}

boost::shared_ptr<LuaItemScript> CommonObjectTable::findScript(TYPE_OF_ITEM_ID Id) {
    SCRIPTTABLE::iterator iterator;
    iterator = m_scripttable.find(Id);

    if (iterator != m_scripttable.end()) {
        return iterator->second;
    } else {
        return boost::shared_ptr<LuaItemScript>();
    }
}

bool CommonObjectTable::find(TYPE_OF_ITEM_ID Id) {
    TABLE::iterator iterator;
    iterator = m_table.find(Id);

    if (iterator == m_table.end()) {
        return false;
    } else {
        return true;
    }
}



void CommonObjectTable::clearOldTable() {
    m_table.clear();
}


CommonObjectTable::~CommonObjectTable() {
    clearOldTable();
}


