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

#include <sys/types.h>
#include "Character.hpp"
#include <boost/unordered_map.hpp>
#include "data/Table.hpp"

class RaceSizeTable: public Table {
public:
    RaceSizeTable();
    ~RaceSizeTable();

    uint8_t getRelativeSize(Character::race_type race, uint16_t size);

    inline bool isDataOk() {
        return m_dataOk;
    }

private:
    virtual void reload();

    bool m_dataOk;
    void clearOldTable();

    typedef boost::unordered_map<uint16_t, uint16_t> TABLE;
    TABLE minsizes;
    TABLE maxsizes;
};

#endif

