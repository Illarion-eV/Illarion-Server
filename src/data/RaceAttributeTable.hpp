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

#ifndef _RACE_ATTRIBUTE_TABLE_HPP_
#define _RACE_ATTRIBUTE_TABLE_HPP_

#include "Character.hpp"
#include "data/StructTable.hpp"
#include "types.hpp"

struct RaceAttributeStruct {
    uint16_t minSize;
    uint16_t maxSize;
};

class RaceAttributeTable : public StructTable<uint16_t, RaceAttributeStruct> {
public:
    virtual std::string getTableName() override;
    virtual std::vector<std::string> getColumnNames() override;
    virtual uint16_t assignId(const Database::ResultTuple &row) override;
    virtual RaceAttributeStruct assignTable(const Database::ResultTuple &row) override;
    uint8_t getRelativeSize(TYPE_OF_RACE_ID race, uint16_t size);
};

#endif

