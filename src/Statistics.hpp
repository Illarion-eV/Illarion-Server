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

#include <string>
#include <limits>
#include <vector>
#include <unordered_map>

namespace Statistic {

class Statistics {
public:
    static Statistics &getInstance();

    void startTimer(const std::string &type);
    // not thread-safe, collect statistics in main thread only for now
    void stopTimer(const std::string &type);

    // not thread-safe, collect statistics in main thread only for now
    void logTime(const std::string& type, int duration);
private:
    Statistics();
    Statistics(const Statistics &) = delete;
    Statistics &operator=(const Statistics &) = delete;

    typedef std::unordered_map<int, int> BinToCount;
    typedef std::vector<BinToCount> PlayerData;
    typedef std::vector<PlayerData> StatTypes;

    static Statistics *instance;
    
    int versionId;

    std::unordered_map<std::string, int> types;

    StatTypes statisticsDB;
    StatTypes statistics;
    StatTypes statisticsSave;
    
    std::vector<long> startTimes;
    static const long SAVE_INTERVAL = 60000; // one minute
    static const int DEFAULT_PLAYERS = 50;
    long lastSaveTime;

    void loadTypes();
    int typeToInt(const std::string &type);
    void load();
    void save();
    int getVersionId();
    long getMillisecondsSinceEpoch() const;
};

}

#endif

