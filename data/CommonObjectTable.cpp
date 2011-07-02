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


#include "db/ConnectionManager.hpp"
#include "CommonObjectTable.hpp"
#include <iostream>
#include "World.hpp"
#include "script/LuaItemScript.hpp"
#include <boost/shared_ptr.hpp>
#include "Logger.hpp"

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
        ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();

        std::vector<TYPE_OF_ITEM_ID> ids;
        std::vector<TYPE_OF_WEIGHT> weights;
        std::vector<TYPE_OF_VOLUME> volumes;
        std::vector<TYPE_OF_AGEINGSPEED> agingspeeds;
        std::vector<TYPE_OF_ITEM_ID> objectafterrot;
        std::vector<bool> isStackable;
        std::vector<bool> rotsininventory;
        std::vector<TYPE_OF_BRIGHTNESS> brightnesses;
        std::vector<TYPE_OF_WORTH> worth;
        std::vector<std::string> scriptname;
        di::isnull_vector<std::vector<std::string> > n_scriptname(scriptname);

        std::map<TYPE_OF_ITEM_ID, bool> assigned; // map item id to whether an infinite rot has been assigned
        std::map<TYPE_OF_ITEM_ID, bool> visited;  // map item id to whether it has been visited already in calcInfiniteRot

        size_t rows = di::select_all<
                      di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Varchar, di::Boolean, di::Boolean, di::Integer, di::Integer
                      >(transaction, ids, volumes, weights, agingspeeds, objectafterrot, n_scriptname, isStackable, rotsininventory, brightnesses, worth,
                        "SELECT com_itemid, com_volume, com_weight, com_agingspeed, com_objectafterrot, com_script, com_stackable, com_rotsininventory, com_brightness, com_worth FROM common");

        if (rows > 0) {
            clearOldTable();

            for (size_t i = 0; i < rows; ++i) {
                CommonStruct temprecord;
                temprecord.id = ids[i];
                temprecord.Weight = weights[i];
                temprecord.Volume = volumes[i];
                temprecord.AgeingSpeed = agingspeeds[i];
                temprecord.ObjectAfterRot = objectafterrot[i];
                temprecord.isStackable = isStackable[i];
                temprecord.rotsInInventory = rotsininventory[i];
                temprecord.Brightness = brightnesses[i];
                temprecord.Worth = worth[i];

                if (!n_scriptname.var[i]) {
                    try {
                        boost::shared_ptr<LuaItemScript> tmpScript(new LuaItemScript(scriptname[i] , temprecord));
                        m_scripttable[ ids[i] ] = tmpScript;
                    } catch (ScriptException &e) {
                        Logger::writeError("scripts", "Error while loading script: " + scriptname[i] + ":\n" + e.what() + "\n");
                    }
                }

                m_table[ ids[i] ] = temprecord;
                visited.insert(std::pair<TYPE_OF_ITEM_ID, bool>(ids[i],false));
                assigned.insert(std::pair<TYPE_OF_ITEM_ID, bool>(ids[i],false));
            }

            // calculate infinite rot for map export
            for (size_t i = 0; i < rows; ++i) {
                if (!assigned[ ids[i] ]) {
                    calcInfiniteRot(ids[i], visited, assigned);
                }
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


