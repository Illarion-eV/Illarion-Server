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

#ifndef _STATISTICS_HPP_
#define _STATISTICS_HPP_

#include <limits>
#include <vector>

namespace Statistic {

struct DataSet {
    long samples = 0;
    long time = 0;
    long timeSquared = 0;
    int timeMin = std::numeric_limits<int>::max();
    int timeMax = 0;
};

class Statistics {
public:
    enum Type {
        cycle,
        player,
        monster,
        npc,
        TYPE_COUNT
    };

    static Statistics &getInstance();

    void startTimer(Type type);
    void stopTimer(Type type);

private:
    Statistics();
    Statistics(const Statistics &) = delete;
    Statistics &operator=(const Statistics &) = delete;

    typedef std::vector<DataSet> PlayerData;
    typedef std::vector<PlayerData> StatTypes;

    static Statistics *instance;
    int versionId;
    StatTypes statistics;
    std::vector<long> startTimes;
    static const long SAVE_INTERVAL = 60000; // one minute
    long lastSaveTime;
    
    void load();
    void save();
    int getVersionId();
    long getMillisecondsSinceEpoch() const;
};

}

#endif

