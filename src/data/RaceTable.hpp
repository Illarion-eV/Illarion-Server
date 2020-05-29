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

#ifndef _RACE_ATTRIBUTE_TABLE_HPP_
#define _RACE_ATTRIBUTE_TABLE_HPP_

#include "Character.hpp"
#include "data/StructTable.hpp"
#include "types.hpp"


class RaceTable : public StructTable<uint16_t, RaceStruct> {
public:
    std::string getTableName() override;
    std::vector<std::string> getColumnNames() override;
    uint16_t assignId(const Database::ResultTuple &row) override;
    RaceStruct assignTable(const Database::ResultTuple &row) override;
    uint8_t getRelativeSize(TYPE_OF_RACE_ID race, uint16_t size) const;
    bool isBaseAttributeInLimits(TYPE_OF_RACE_ID race, Character::attributeIndex attribute, Attribute::attribute_t value) const;
    uint8_t getMaxAttributePoints(TYPE_OF_RACE_ID race) const;
};

#endif

