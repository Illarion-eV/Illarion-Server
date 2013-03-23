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

#include "data/NamesObjectTable.hpp"

std::string NamesObjectTable::getTableName() {
    return "itemname";
}

std::vector<std::string> NamesObjectTable::getColumnNames() {
    return {
        "itn_itemid",
        "itn_german",
        "itn_english"
    };
}

TYPE_OF_ITEM_ID NamesObjectTable::assignId(const Database::ResultTuple &row) {
    return row["itn_itemid"].as<TYPE_OF_ITEM_ID>();
}

NamesStruct NamesObjectTable::assignTable(const Database::ResultTuple &row) {
    NamesStruct names;
    names.German = row["itn_german"].as<TYPE_OF_GERMAN>();
    names.English = row["itn_english"].as<TYPE_OF_ENGLISH>();
    return names;
}

