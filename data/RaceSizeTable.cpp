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

#include "data/RaceSizeTable.hpp"

#include "db/SelectQuery.hpp"
#include "db/Result.hpp"

#include "types.hpp"


RaceSizeTable::RaceSizeTable() : m_dataOk(false) {
    reload();
}

RaceSizeTable::~RaceSizeTable() {
    clearOldTable();
}

void RaceSizeTable::reload() {
#ifdef DataConnect_DEBUG
    std::cout<<"Trying to reload RaceSizeTable!"<<std::endl;
#endif

    try {
        Database::SelectQuery query;
        query.addColumn("raceattr", "id");
        query.addColumn("raceattr", "minbodyheight");
        query.addColumn("raceattr", "maxbodyheight");
        query.addServerTable("raceattr");

        Database::Result results = query.execute();
        std::vector<uint16_t>minheight;
        std::vector<uint16_t>maxheight;
        std::vector<uint16_t>raceid;

        if (!results.empty()) {
            uint16_t currentID;

            for (Database::ResultConstIterator itr = results.begin();
                 itr != results.end(); ++itr) {

                currentID = (uint16_t)((*itr)["id"].as<int32_t>());
                minsizes[currentID] = (uint16_t)((*itr)["minbodyheight"].as<int32_t>());
                maxsizes[currentID] = (uint16_t)((*itr)["maxbodyheight"].as<int32_t>());
            }
        }


        m_dataOk = true;

#ifdef DataConnect_DEBUG
        std::cout << "loaded " << rows << " rows into RaceSizeTable" << std::endl;
#endif
    } catch (...) {
        m_dataOk = false;
    }
}

uint8_t RaceSizeTable::getRelativeSize(Character::race_type race, uint16_t size) {
    // relative size is between 50 and 120 (in percent) and a linear interploation between min and max size
    uint16_t maxsize, minsize;
    TABLE::iterator iter;
    iter = minsizes.find(static_cast<uint16_t>(race));

    if (iter == minsizes.end()) {
        return 100;
    } else {
        minsize = iter->second;
    }

    iter = maxsizes.find(static_cast<uint16_t>(race));

    if (iter == maxsizes.end()) {
        return 100;
    } else {
        maxsize = iter->second;
    }

    if (size >= minsize && size <= maxsize) {
        return (uint8_t)((40*(size-minsize))/(maxsize-minsize)+80);
    } else {
        return 100;
    }
}

void RaceSizeTable::clearOldTable() {
    minsizes.clear();
    maxsizes.clear();
}

