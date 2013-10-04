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
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU  General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _TILES_MODIFICATOR_TABLE_HPP_
#define _TILES_MODIFICATOR_TABLE_HPP_

#include "data/StructTable.hpp"
#include "types.hpp"
#include "TableStructs.hpp"

class TilesModificatorTable : public StructTable<TYPE_OF_ITEM_ID, TilesModificatorStruct> {
public:
    virtual std::string getTableName() override;
    virtual std::vector<std::string> getColumnNames() override;
    virtual TYPE_OF_ITEM_ID assignId(const Database::ResultTuple &row) override;
    virtual TilesModificatorStruct assignTable(const Database::ResultTuple &row) override;
    bool nonPassable(TYPE_OF_ITEM_ID id);
};

#endif

