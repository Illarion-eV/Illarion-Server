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

std::string CommonObjectTable::getTableName() {
    return "common";
}

std::vector<std::string> CommonObjectTable::getColumnNames() {
    return {
        "com_itemid",
        "com_weight",
        "com_agingspeed",
        "com_objectafterrot",
        "com_rotsininventory",
        "com_script",
        "com_brightness",
        "com_worth",
        "com_buystack",
        "com_maxstack"
    };
}

TYPE_OF_ITEM_ID CommonObjectTable::assignId(const Database::ResultTuple &row) {
    return row["com_itemid"].as<TYPE_OF_ITEM_ID>();
}

CommonStruct CommonObjectTable::assignTable(const Database::ResultTuple &row) {
    CommonStruct common;
    common.id = assignId(row);
    common.Weight = TYPE_OF_WEIGHT(row["com_weight"].as<int16_t>());
    common.AgeingSpeed = TYPE_OF_AGINGSPEED(row["com_agingspeed"].as<int16_t>());
    common.ObjectAfterRot = row["com_objectafterrot"].as<TYPE_OF_ITEM_ID>();
    common.rotsInInventory = row["com_rotsininventory"].as<bool>();
    common.Brightness = TYPE_OF_BRIGHTNESS(row["com_brightness"].as<int16_t>());
    common.Worth = row["com_worth"].as<TYPE_OF_WORTH>();
    common.BuyStack = row["com_buystack"].as<TYPE_OF_BUY_STACK>();
    common.MaxStack = row["com_maxstack"].as<TYPE_OF_MAX_STACK>();
    return common;
}

std::string CommonObjectTable::assignScriptName(const Database::ResultTuple &row) {
    return row["com_script"].as<std::string>("");
}

QuestNodeTable::TABLE_ITRS CommonObjectTable::getQuestScripts() {
    return QuestNodeTable::getInstance()->getItemNodes();
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

