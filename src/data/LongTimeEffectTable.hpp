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

#ifndef _LONG_TIME_EFFECT_TABLE_HPP_
#define _LONG_TIME_EFFECT_TABLE_HPP_

#include "data/ScriptStructTable.hpp"
#include "TableStructs.hpp"
#include "script/LuaLongTimeEffectScript.hpp"

class LongTimeEffectTable : public ScriptStructTable<uint16_t, LongTimeEffectStruct, LuaLongTimeEffectScript> {
public:
    std::string getTableName() override;
    std::vector<std::string> getColumnNames() override;
    uint16_t assignId(const Database::ResultTuple &row) override;
    LongTimeEffectStruct assignTable(const Database::ResultTuple &row) override;
    std::string assignScriptName(const Database::ResultTuple &row) override;
};

#endif

