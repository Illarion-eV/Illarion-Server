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

#include "Statistics.hpp"
#include "version.hpp"
#include "db/ConnectionManager.hpp"
#include "db/SelectQuery.hpp"
#include "db/InsertQuery.hpp"
#include "db/UpdateQuery.hpp"
#include "db/Query.hpp"
#include "World.hpp"
#include "Logger.hpp"
#include <chrono>
#include <algorithm>
#include <thread>
#include <stdexcept>

namespace Statistic {

Statistics *Statistics::instance = nullptr;

Statistics::Statistics() {
/*
    loadTypes();
    auto typesCount = types.size();

    statisticsDB.resize(typesCount);
    statistics.resize(typesCount);
    statisticsSave.resize(typesCount);
    startTimes.resize(typesCount);

    for (size_t i = 0; i < typesCount; ++i) {
        for (int j = 0; j < DEFAULT_PLAYERS; ++j) {
            statisticsDB[i].emplace_back();
            statistics[i].emplace_back();
            statisticsSave[i].emplace_back();
        }
    }

    versionId = getVersionId();
    lastSaveTime = getMillisecondsSinceEpoch();
    load();
*/
}

Statistics &Statistics::getInstance() {
    if (!instance) {
        instance = new Statistics();
    }
    
    return *instance;
}

void Statistics::startTimer(const std::string &type) {
/*
    int intType;

    try {
        intType = typeToInt(type);
    } catch (std::out_of_range &e) {
        return;
    }

    startTimes[intType] = getMillisecondsSinceEpoch();
*/
}

void Statistics::stopTimer(const std::string &type) {
/*
    int intType;

    try {
        intType = typeToInt(type);
    } catch (std::out_of_range &e) {
        return;
    }   

    long now = getMillisecondsSinceEpoch();
    int duration = static_cast<int>(now - startTimes[intType]);
    startTimes[intType] = now;
    auto playersOnline = World::get()->Players.size();

    while (statistics[intType].size() <= playersOnline) {
        statistics[intType].emplace_back();
    }
    
    ++statistics[intType][playersOnline][duration];

    if (now - lastSaveTime > SAVE_INTERVAL) {
        lastSaveTime = now;

        statistics.swap(statisticsSave);
        std::thread t(&Statistics::save, this);
        t.detach();
    }
*/
}

void Statistics::logTime(const std::string& type, int duration) {
/*
    int intType;

    try {
        intType = typeToInt(type);
    } catch (std::out_of_range &e) {
        return;
    }
    auto playersOnline = World::get()->Players.size();

    while (statistics[intType].size() <= playersOnline) {
        statistics[intType].emplace_back();
    }

    ++statistics[intType][playersOnline][duration];

    long now = getMillisecondsSinceEpoch();
    if (now - lastSaveTime > SAVE_INTERVAL) {
        lastSaveTime = now;

        statistics.swap(statisticsSave);
        std::thread t(&Statistics::save, this);
        t.detach();
    }
*/
}

int Statistics::typeToInt(const std::string &type) {
    const auto it = types.find(type);

    if (it != types.end()) {
        return it->second;
    } else {
        const auto unknownIt = types.find("unknown");

        if (unknownIt == types.end()) {
            throw std::out_of_range("unknown statistic type");
        }

        return unknownIt->second;
    }
}

int Statistics::getVersionId() {
    using namespace Database;
    auto connection = ConnectionManager::getInstance().getConnection();

    SelectQuery query(connection);
    query.addColumn("versions", "version_id");
    query.addEqualCondition<std::string>("versions", "version_name", SERVER_VERSION);
    query.addServerTable("versions");

    auto result = query.execute();

    if (!result.empty()) {
        const auto &row = result.front();
        return row["version_id"].as<int>();
    }

    InsertQuery insQuery(connection);
    const auto nameColumn = insQuery.addColumn("version_name");
    insQuery.addServerTable("versions");
    insQuery.addValue<std::string>(nameColumn, SERVER_VERSION);
    insQuery.execute();

    Query idQuery(connection, "SELECT lastval();");
    auto idResult = idQuery.execute();
    const auto &row = idResult.front();
    return row["lastval"].as<int>();
}

void Statistics::loadTypes() {
    using namespace Database;

    SelectQuery query;
    query.addColumn("statistics_types", "stat_type_id");
    query.addColumn("statistics_types", "stat_type_name");
    query.addServerTable("statistics_types");

    auto result = query.execute();

    for (const auto &row : result) {
        types.emplace(row["stat_type_name"].as<std::string>(), row["stat_type_id"].as<int>());
    }
}

void Statistics::load() {
    using namespace Database;

    SelectQuery query;
    query.addColumn("statistics", "stat_type");
    query.addColumn("statistics", "stat_players");
    query.addColumn("statistics", "stat_bin");
    query.addColumn("statistics", "stat_count");
    query.addEqualCondition<int>("statistics", "stat_version", versionId);
    query.addServerTable("statistics");

    auto result = query.execute();

    for (const auto &row : result) {
        int type = row["stat_type"].as<int>();
        int players_online = row["stat_players"].as<int>();
        int bin = row["stat_bin"].as<int>();
        int count = row["stat_count"].as<int>();

        while (statisticsDB[type].size() <= static_cast<size_t>(players_online)) {
            statisticsDB[type].emplace_back();
        }

        statisticsDB[type][players_online][bin] = count;
    }
}

void Statistics::save() {
    using namespace Database;

    auto connection = ConnectionManager::getInstance().getConnection();

    try {
        InsertQuery insertQuery(connection);
        const auto versionColumn = insertQuery.addColumn("stat_version");
        const auto typeColumn = insertQuery.addColumn("stat_type");
        const auto playersColumn = insertQuery.addColumn("stat_players");
        const auto binColumn = insertQuery.addColumn("stat_bin");
        const auto countColumn = insertQuery.addColumn("stat_count");
        insertQuery.addServerTable("statistics");

        int currentType = 0;
        for (auto &types : statisticsSave) {
            auto &typeDB = statisticsDB[currentType];
            int players = 0;

            for (auto &bins : types) {
                auto &playersDB = typeDB[players];

                for (const auto &bin : bins) {
                    auto dbIterator = playersDB.find(bin.first);

                    if (dbIterator == playersDB.end()) {
                        if (bin.second > 0) {
                            playersDB.insert(bin);
                            
                            insertQuery.addValue<int>(versionColumn, versionId);
                            insertQuery.addValue<int>(typeColumn, currentType);
                            insertQuery.addValue<int>(playersColumn, players);
                            insertQuery.addValue<int>(binColumn, bin.first);
                            insertQuery.addValue<int>(countColumn, bin.second);
                        }
                    } else {
                        dbIterator->second += bin.second;
                        
                        Database::UpdateQuery query;
                        query.addAssignColumn<int>("stat_count", dbIterator->second);
                        query.addEqualCondition<int>("statistics", "stat_version", versionId);
                        query.addEqualCondition<int>("statistics", "stat_type", currentType);
                        query.addEqualCondition<int>("statistics", "stat_players", players);
                        query.addEqualCondition<int>("statistics", "stat_bin", bin.first);
                        query.addServerTable("statistics");
                        query.execute();
                    }
                }

                bins.clear();
                ++players;
            }

            ++currentType;
        }

        insertQuery.execute();
    } catch (std::exception &e) {
        Logger::error(LogFacility::Other) << "saving statistics caught exception: " << e.what() << Log::end;
        connection->rollbackTransaction();
    }

}

long Statistics::getMillisecondsSinceEpoch() const {
    using namespace std::chrono;
    return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
}

}

