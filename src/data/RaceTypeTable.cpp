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

#include <algorithm>
#include "data/RaceTypeTable.hpp"
#include "db/Connection.hpp"
#include "db/ConnectionManager.hpp"
#include "db/SelectQuery.hpp"
#include "db/Result.hpp"
#include "Logger.hpp"
#include "Random.hpp"

RaceTypeTable::RaceTypeTable() {
    Logger::info(LogFacility::Other) << "RaceTypeTable::constructor" << Log::end;

    try {
        using namespace Database;
        PConnection connection = ConnectionManager::getInstance().getConnection();
        connection->beginTransaction();

        {
            SelectQuery query(connection);
            query.addColumn("race_types", "rt_race_id");
            query.addColumn("race_types", "rt_type_id");
            query.addServerTable("race_types");

            Database::Result results = query.execute();

            for (const auto &row : results) {
                const auto id = row["rt_race_id"].as<TYPE_OF_RACE_ID>();
                const auto type = row["rt_type_id"].as<TYPE_OF_RACE_TYPE_ID>();

                table[id][type];
            }
        }

        {
            SelectQuery query(connection);
            query.addColumn("race_hair", "rh_race_id");
            query.addColumn("race_hair", "rh_type_id");
            query.addColumn("race_hair", "rh_hair_id");
            query.addServerTable("race_hair");

            Database::Result results = query.execute();

            for (const auto &row : results) {
                const auto id = row["rh_race_id"].as<TYPE_OF_RACE_ID>();
                const auto type = row["rh_type_id"].as<TYPE_OF_RACE_TYPE_ID>();
                const auto hair = row["rh_hair_id"].as<uint16_t>();

                table[id][type].hair.push_back(hair);
            }
        }

        {
            SelectQuery query(connection);
            query.addColumn("race_beard", "rb_race_id");
            query.addColumn("race_beard", "rb_type_id");
            query.addColumn("race_beard", "rb_beard_id");
            query.addServerTable("race_beard");

            Database::Result results = query.execute();

            for (const auto &row : results) {
                const auto id = row["rb_race_id"].as<TYPE_OF_RACE_ID>();
                const auto type = row["rb_type_id"].as<TYPE_OF_RACE_TYPE_ID>();
                const auto beard = row["rb_beard_id"].as<uint16_t>();

                table[id][type].beard.push_back(beard);
            }
        }

        {
            SelectQuery query(connection);
            query.addColumn("race_hair_colour", "rhc_race_id");
            query.addColumn("race_hair_colour", "rhc_type_id");
            query.addColumn("race_hair_colour", "rhc_red");
            query.addColumn("race_hair_colour", "rhc_green");
            query.addColumn("race_hair_colour", "rhc_blue");
            query.addColumn("race_hair_colour", "rhc_alpha");
            query.addServerTable("race_hair_colour");

            Database::Result results = query.execute();

            for (const auto &row : results) {
                const auto id = row["rhc_race_id"].as<TYPE_OF_RACE_ID>();
                const auto type = row["rhc_type_id"].as<TYPE_OF_RACE_TYPE_ID>();
                const auto red = uint8_t(row["rhc_red"].as<uint16_t>());
                const auto green = uint8_t(row["rhc_green"].as<uint16_t>());
                const auto blue = uint8_t(row["rhc_blue"].as<uint16_t>());
                const auto alpha = uint8_t(row["rhc_alpha"].as<uint16_t>());

                table[id][type].hairColour.emplace_back(red, green, blue, alpha);
            }
        }

        {
            SelectQuery query(connection);
            query.addColumn("race_skin_colour", "rsc_race_id");
            query.addColumn("race_skin_colour", "rsc_type_id");
            query.addColumn("race_skin_colour", "rsc_red");
            query.addColumn("race_skin_colour", "rsc_green");
            query.addColumn("race_skin_colour", "rsc_blue");
            query.addColumn("race_skin_colour", "rsc_alpha");
            query.addServerTable("race_skin_colour");

            Database::Result results = query.execute();

            for (const auto &row : results) {
                const auto id = row["rsc_race_id"].as<TYPE_OF_RACE_ID>();
                const auto type = row["rsc_type_id"].as<TYPE_OF_RACE_TYPE_ID>();
                const auto red = uint8_t(row["rsc_red"].as<uint16_t>());
                const auto green = uint8_t(row["rsc_green"].as<uint16_t>());
                const auto blue = uint8_t(row["rsc_blue"].as<uint16_t>());
                const auto alpha = uint8_t(row["rsc_alpha"].as<uint16_t>());

                table[id][type].skinColour.emplace_back(red, green, blue, alpha);
            }
        }

        connection->commitTransaction();
        Logger::info(LogFacility::Other) << "Loaded " << table.size() << " race subtypes." << Log::end;
        dataOK = true;
    } catch (std::exception &e) {
        Logger::error(LogFacility::Other) << "Exception in RaceTypeTable::constructor: " << e.what() << Log::end;
        dataOK = false;
    }
}

const RaceConfiguration RaceTypeTable::getRandomRaceConfiguration(TYPE_OF_RACE_ID race) const {
    RaceConfiguration raceConfiguration;
    const auto subTypeCount = table.count(race);

    if (subTypeCount == 0) {
        Logger::error(LogFacility::Script) << "No subtypes defined for race " << race << Log::end;
        return raceConfiguration;
    }

    int selectedSubTypeNumber = Random::uniform(0, subTypeCount - 1);
    auto it = table.at(race).begin();

    // sub types do not need to be sequentiell
    for (int i = 0; i < selectedSubTypeNumber; ++i) {
        ++it;
    }

    const auto subType = it->first;
    const auto &raceType = it->second;
    raceConfiguration.subType = subType;

    if (raceType.hair.size() > 0) {
        const auto randomHairId = Random::uniform(0, raceType.hair.size() - 1);
        raceConfiguration.hair = raceType.hair[randomHairId];
    }

    if (raceType.beard.size() > 0) {
        const auto randomBeardId = Random::uniform(0, raceType.beard.size() - 1);
        raceConfiguration.beard = raceType.beard[randomBeardId];
    }

    if (raceType.hairColour.size() > 0) {
        const auto randomHairColourId = Random::uniform(0, raceType.hairColour.size() - 1);
        raceConfiguration.hairColour = raceType.hairColour[randomHairColourId];
    }

    if (raceType.skinColour.size() > 0) {
        const auto randomSkinColourId = Random::uniform(0, raceType.skinColour.size() - 1);
        raceConfiguration.skinColour = raceType.skinColour[randomSkinColourId];
    }

    return raceConfiguration;
}

bool RaceTypeTable::isHairAvailable(TYPE_OF_RACE_ID race, TYPE_OF_RACE_TYPE_ID type, uint16_t hair) const {
    try {
        const auto &availableHair = table.at(race).at(type).hair;
        return std::find(availableHair.begin(), availableHair.end(), hair) != availableHair.end();
    } catch (std::out_of_range &) {
        return false;
    }
}

bool RaceTypeTable::isBeardAvailable(TYPE_OF_RACE_ID race, TYPE_OF_RACE_TYPE_ID type, uint16_t beard) const {
    try {
        const auto &availableBeards = table.at(race).at(type).beard;
        return std::find(availableBeards.begin(), availableBeards.end(), beard) != availableBeards.end();
    } catch (std::out_of_range &) {
        return false;
    }
}
