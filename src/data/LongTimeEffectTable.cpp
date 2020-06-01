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

#include "data/LongTimeEffectTable.hpp"

auto LongTimeEffectTable::getTableName() -> std::string {
    return "longtimeeffects";
}

auto LongTimeEffectTable::getColumnNames() -> std::vector<std::string> {
    return {"lte_effectid", "lte_effectname", "lte_scriptname"};
}

auto LongTimeEffectTable::assignId(const Database::ResultTuple &row) -> uint16_t {
    return row["lte_effectid"].as<uint16_t>();
}

auto LongTimeEffectTable::assignTable(const Database::ResultTuple &row) -> LongTimeEffectStruct {
    LongTimeEffectStruct lte;
    lte.effectid = assignId(row);
    lte.effectname = row["lte_effectid"].as<std::string>();
    return lte;
}

auto LongTimeEffectTable::assignScriptName(const Database::ResultTuple &row) -> std::string {
    return row["lte_scriptname"].as<std::string>("");
}
