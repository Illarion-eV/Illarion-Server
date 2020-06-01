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

#include "data/ContainerObjectTable.hpp"

auto ContainerObjectTable::getTableName() -> std::string { return "container"; }

auto ContainerObjectTable::getColumnNames() -> std::vector<std::string> { return {"con_itemid", "con_slots"}; }

auto ContainerObjectTable::assignId(const Database::ResultTuple &row) -> TYPE_OF_ITEM_ID {
    return row["con_itemid"].as<TYPE_OF_ITEM_ID>();
}

auto ContainerObjectTable::assignTable(const Database::ResultTuple &row) -> TYPE_OF_CONTAINERSLOTS {
    return row["con_slots"].as<TYPE_OF_CONTAINERSLOTS>();
}
