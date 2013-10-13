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
/*
 * TODO: The handling of the tile modificators is extremly unsafe. Errors in the
 *       database will corrupt the data easily und uncontrolable.
 */

#include "data/TilesModificatorTable.hpp"
#include "constants.hpp"

std::string TilesModificatorTable::getTableName() {
    return "tilesmodificators";
}

std::vector<std::string> TilesModificatorTable::getColumnNames() {
    return {
        "tim_itemid",
        "tim_isnotpassable",
        "tim_isnottransparent",
        "tim_isnotpenetrateable",
        "tim_specialitem",
        "tim_groundlevel",
        "tim_makepassable"
    };
}

TYPE_OF_ITEM_ID TilesModificatorTable::assignId(const Database::ResultTuple &row) {
    return row["tim_itemid"].as<TYPE_OF_ITEM_ID>();
}

TilesModificatorStruct TilesModificatorTable::assignTable(const Database::ResultTuple &row) {
    TilesModificatorStruct modStruct;
    modStruct.Modificator = uint8_t(row["tim_groundlevel"].as<uint16_t>());
    modStruct.Modificator |= row["tim_isnotpassable"].as<bool>() ? FLAG_PASSABLE : 0;
    modStruct.Modificator |= row["tim_isnottransparent"].as<bool>() ? FLAG_TRANSPARENT : 0;
    modStruct.Modificator |= row["tim_isnotpenetrateable"].as<bool>() ? FLAG_TRANSPARENT : 0;
    modStruct.Modificator |= row["tim_specialitem"].as<bool>() ? FLAG_SPECIALITEM : 0;
    modStruct.Modificator |= row["tim_makepassable"].as<bool>() ? FLAG_MAKEPASSABLE : 0;
    return modStruct;
}

bool TilesModificatorTable::nonPassable(TYPE_OF_ITEM_ID id) {
    if (exists(id)) {
        const auto &modStruct = (*this)[id];
        return ((modStruct.Modificator & FLAG_PASSABLE) != 0) && ((modStruct.Modificator & FLAG_MAKEPASSABLE) == 0);
    }

    return false;
}

