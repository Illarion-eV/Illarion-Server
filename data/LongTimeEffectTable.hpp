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

#ifndef _LONG_TIME_EFFECT_TABLE_HPP_
#define _LONG_TIME_EFFECT_TABLE_HPP_

#include <string>
#include <boost/unordered_map.hpp>
#include "TableStructs.hpp"

class LongTimeEffectTable {
public:
    bool find(uint16_t effectId, LongTimeEffectStruct &ret);
    bool find(std::string effectname, LongTimeEffectStruct &ret);

    LongTimeEffectTable();
    ~LongTimeEffectTable();

    inline bool dataOK() {
        return m_dataOK;
    }

private:
    typedef boost::unordered_map<uint16_t, LongTimeEffectStruct> TABLE;
    TABLE m_table;

    void clearOldTable();

    void reload();
    bool m_dataOK;
};
#endif

