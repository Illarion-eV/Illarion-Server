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

#include "data/LongTimeEffectTable.hpp"

std::string LongTimeEffectTable::getTableName() {
    return "longtimeeffects";
}

std::vector<std::string> LongTimeEffectTable::getColumnNames() {
    return {
        "lte_effectid",
        "lte_effectname",
        "lte_scriptname"
    };
}

uint16_t LongTimeEffectTable::assignId(const Database::ResultTuple &row) {
    return row["lte_effectid"].as<uint16_t>();
}

LongTimeEffectStruct LongTimeEffectTable::assignTable(const Database::ResultTuple &row) {
    LongTimeEffectStruct lte;
    lte.effectid = assignId(row);
    lte.effectname = row["lte_effectid"].as<std::string>();
    return lte;
}

std::string LongTimeEffectTable::assignScriptName(const Database::ResultTuple &row) {
    return row["lte_scriptname"].as<std::string>("");
}

