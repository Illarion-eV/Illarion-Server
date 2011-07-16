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

#include "data/TilesTable.hpp"

#include <iostream>
#include <string>

#include "db/SelectQuery.hpp"
#include "db/Result.hpp"

#include "script/LuaTileScript.hpp"

#include "constants.hpp"
#include "TableStructs.hpp"
#include "Logger.hpp"

TilesTable::TilesTable() : m_dataOK(false) {
    reload();
}


void TilesTable::reload() {
#ifdef DataConnect_DEBUG
    std::cout << "TilesTable: reload" << std::endl;
#endif

    try {
        Database::SelectQuery query;
        query.addColumn("tiles", "til_id");
        query.addColumn("tiles", "til_isnotpassable");
        query.addColumn("tiles", "til_isnottransparent");
        query.addColumn("tiles", "til_isnotpenetrateable");
        query.addColumn("tiles", "til_specialtile");
        query.addColumn("tiles", "til_groundlevel");
        query.addColumn("tiles", "til_german");
        query.addColumn("tiles", "til_english");
        query.addColumn("tiles", "til_french");
        query.addColumn("tiles", "til_walkingcost");
        query.addColumn("tiles", "til_script");
        query.addServerTable("tiles");

        Database::Result results = query.execute();

        if (!results.empty()) {
            clearOldTable();
            TilesStruct temprecord;
            std::string scriptname;
            TYPE_OF_ITEM_ID tileId;

            for (Database::Result::ConstIterator itr = results.begin();
                 itr != results.end(); ++itr) {
                tileId = (*itr)["til_id"].as<TYPE_OF_ITEM_ID>();
                temprecord.flags = (uint8_t)((*itr)["til_groundlevel"].as<uint16_t>());
                temprecord.flags |= (*itr)["til_isnotpassable"].as<bool>() ? FLAG_PASSABLE : 0;
                temprecord.flags |= (*itr)["til_isnottransparent"].as<bool>() ? FLAG_TRANSPARENT : 0;
                temprecord.flags |= (*itr)["til_isnotpenetrateable"].as<bool>() ? FLAG_TRANSPARENT : 0;
                temprecord.flags |= (*itr)["til_specialtile"].as<bool>() ? FLAG_SPECIALITEM : 0;
                temprecord.German = (*itr)["til_german"].as<std::string>();
                temprecord.English = (*itr)["til_english"].as<std::string>();
                temprecord.French = (*itr)["til_french"].as<std::string>();
                temprecord.walkingCost = (TYPE_OF_WALKINGCOST)((*itr)["til_walkingcost"].as<int16_t>());

                if (!(*itr)["til_script"].is_null()) {
                    scriptname = (*itr)["til_script"].as<std::string>();

                    if (!scriptname.empty()) {
                        try {
                            boost::shared_ptr<LuaTileScript> script(new LuaTileScript(scriptname, temprecord));
                            temprecord.script = script;
                        } catch (ScriptException &e) {
                            Logger::writeError("scripts", "Error while loading script: " + scriptname + ":\n" + e.what() + "\n");
                        }
                    }
                }

                m_table[tileId] = temprecord;
            }

            m_dataOK = true;
        } else {
            m_dataOK = false;
        }


#ifdef DataConnect_DEBUG
        std::cout << "loaded " << rows << " rows into TilesTable" << std::endl;
#endif

    } catch (std::exception &e) {
        std::cout << "exception in tiles loading: " << e.what() << std::endl;
        m_dataOK = false;
    }


}

bool TilesTable::find(TYPE_OF_ITEM_ID Id, TilesStruct &ret) {
    TABLE::iterator iterator;
    iterator = m_table.find(Id);

    if (iterator == m_table.end()) {
        return false;
    } else {
        ret = (*iterator).second;
        return true;
    }
}

void TilesTable::clearOldTable() {
    m_table.clear();
}

TilesTable::~TilesTable() {
    clearOldTable();
}
