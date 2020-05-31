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

#ifndef RACE_ATTRIBUTE_TABLE_HPP
#define RACE_ATTRIBUTE_TABLE_HPP

#include "Character.hpp"
#include "data/StructTable.hpp"
#include "types.hpp"


class RaceTable : public StructTable<uint16_t, RaceStruct> {
public:
    auto getTableName() -> std::string override;
    auto getColumnNames() -> std::vector<std::string> override;
    auto assignId(const Database::ResultTuple &row) -> uint16_t override;
    auto assignTable(const Database::ResultTuple &row) -> RaceStruct override;
    auto getRelativeSize(TYPE_OF_RACE_ID race, uint16_t size) const -> uint8_t;
    auto isBaseAttributeInLimits(TYPE_OF_RACE_ID race, Character::attributeIndex attribute, Attribute::attribute_t value) const -> bool;
    auto getMaxAttributePoints(TYPE_OF_RACE_ID race) const -> uint8_t;
};

#endif

