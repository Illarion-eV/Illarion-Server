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

#ifndef _RACE_SIZE_TABLE_HPP_
#define _RACE_SIZE_TABLE_HPP_

#include "Character.hpp"
#include "data/StructTable.hpp"
#include "types.hpp"
#include "TableStructs.hpp"

struct RaceSizeStruct {
    uint16_t minSize;
    uint16_t maxSize;
};

class RaceSizeTable : public StructTable<uint16_t, RaceSizeStruct> {
public:
    virtual std::string getTableName() override;
    virtual std::vector<std::string> getColumnNames() override;
    virtual uint16_t assignId(const Database::ResultTuple &row) override;
    virtual RaceSizeStruct assignTable(const Database::ResultTuple &row) override;
    uint8_t getRelativeSize(Character::race_type race, uint16_t size);
};

#endif

