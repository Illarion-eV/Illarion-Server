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

#include "data/CommonObjectTable.hpp"

std::string CommonObjectTable::getTableName() {
    return "items";
}

std::vector<std::string> CommonObjectTable::getColumnNames() {
    return {
        "itm_id",
        "itm_volume",
        "itm_weight",
        "itm_agingspeed",
        "itm_objectafterrot",
        "itm_rotsininventory",
        "itm_script",
        "itm_brightness",
        "itm_worth",
        "itm_buystack",
        "itm_maxstack",
        "itm_name_english",
        "itm_name_german"
    };
}

TYPE_OF_ITEM_ID CommonObjectTable::assignId(const Database::ResultTuple &row) {
    return row["itm_id"].as<TYPE_OF_ITEM_ID>();
}

CommonStruct CommonObjectTable::assignTable(const Database::ResultTuple &row) {
    CommonStruct common;
    common.id = assignId(row);
    common.Volume = row["itm_volume"].as<TYPE_OF_VOLUME>();
    common.Weight = row["itm_weight"].as<TYPE_OF_WEIGHT>();
    common.AgeingSpeed = TYPE_OF_AGINGSPEED(row["itm_agingspeed"].as<int16_t>());
    common.ObjectAfterRot = row["itm_objectafterrot"].as<TYPE_OF_ITEM_ID>();
    common.rotsInInventory = row["itm_rotsininventory"].as<bool>();
    common.Brightness = TYPE_OF_BRIGHTNESS(row["itm_brightness"].as<int16_t>());
    common.Worth = row["itm_worth"].as<TYPE_OF_WORTH>();
    common.BuyStack = row["itm_buystack"].as<TYPE_OF_BUY_STACK>();
    common.MaxStack = row["itm_maxstack"].as<TYPE_OF_MAX_STACK>();
    common.German = row["itm_name_german"].as<TYPE_OF_GERMAN>();
    common.English = row["itm_name_english"].as<TYPE_OF_ENGLISH>();
    return common;
}

std::string CommonObjectTable::assignScriptName(const Database::ResultTuple &row) {
    return row["itm_script"].as<std::string>("");
}

auto CommonObjectTable::getQuestScripts() -> NodeRange {
    return QuestNodeTable::getInstance().getItemNodes();
}

/*
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
*/

