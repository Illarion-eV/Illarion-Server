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

#include "SpawnPoint.hpp"

#include <boost/cstdint.hpp>

#include "db/SelectQuery.hpp"
#include "db/Result.hpp"

#include "Field.hpp"
#include "Logger.hpp"
#include "Monster.hpp"
#include "Random.hpp"
#include "World.hpp"


//! Creates a new SpawnPoint at <pos>
SpawnPoint::SpawnPoint(const position &pos, int Range, uint16_t Spawnrange, uint16_t Min_Spawntime, uint16_t Max_Spawntime, bool Spawnall) : world(World::get()), spawnpos(pos), range(Range), spawnrange(Spawnrange), min_spawntime(Min_Spawntime), max_spawntime(Max_Spawntime), spawnall(Spawnall) {
    nextspawntime = Random::uniform(min_spawntime, max_spawntime);
}

//! Destructor
SpawnPoint::~SpawnPoint() {}

//! add new Monstertyp to SpawnList...
void SpawnPoint::addMonster(const TYPE_OF_CHARACTER_ID &typ, const short int &count) {
    for (auto &spawn : SpawnTypes) {
        if (spawn.typ == typ) {
            // monster type already there... just raise max_count
            spawn.max_count += count;
            return;
        }
    }

    // no entry found... create new one..
    struct SpawnEntryStruct SEtemp;
    SEtemp.typ = typ;
    SEtemp.max_count = count;
    SEtemp.akt_count = 0;
    SpawnTypes.push_back(SEtemp);
}

//! do spawns if possible...
void SpawnPoint::spawn() {
    if (nextspawntime <= 0) {
        //set new spawntime
        nextspawntime = Random::uniform(min_spawntime, max_spawntime);

        // do we want monsters to spawn?
        if (!World::get()->isSpawnEnabled()) {
            return;
        }

        Monster *newmonster;

        // check all monstertyps...
        for (auto &spawn : SpawnTypes) {
            // less monster spawned than we need?
            int num = spawn.max_count - spawn.akt_count;
            if (num > 0) {
                try {
                    // spawn some new baddies :)
                    if (!spawnall) {
                        num = Random::uniform(1, num);
                    }

                    for (int i = 0; i < num; ++i) {
                        position tempPos;
                        //set the new spawnpos in the range of the spawnrange around the spawnpoint
                        tempPos.x = (spawnpos.x - spawnrange) + Random::uniform(0, 2 * spawnrange);
                        tempPos.y = (spawnpos.y - spawnrange) + Random::uniform(0, 2 * spawnrange);
                        tempPos.z = spawnpos.z;

                        //end of setting the new spawnpos
                        try {
                            Field &field = world->walkableFieldNear(tempPos);
                            newmonster = new Monster(spawn.typ, tempPos, this);
                            ++spawn.akt_count;
                            world->newMonsters.push_back(newmonster);
                            field.setPlayer();
                            world->sendCharacterMoveToAllVisiblePlayers(newmonster, NORMALMOVE, 4);
                        } catch (FieldNotFound &) {
                        }
                    }
                } catch (Monster::unknownIDException &) {
                    Logger::error(LogFacility::Other) << "Could not create unknown monster " << spawn.typ << Log::end;
                }
            }
        }
    } else {
        --nextspawntime;
    }
}

void SpawnPoint::dead(const TYPE_OF_CHARACTER_ID &typ) {
    for (auto &spawn : SpawnTypes) {
        if (spawn.typ == typ) {
            spawn.akt_count--;
        }
    }
}

bool SpawnPoint::load(const int &id) {
    try {
        Database::SelectQuery query;
        query.addColumn("spawnpoint_monster", "spm_race");
        query.addColumn("spawnpoint_monster", "spm_count");
        query.addEqualCondition("spawnpoint_monster", "spm_id", id);
        query.addServerTable("spawnpoint_monster");

        Database::Result results = query.execute();

        if (!results.empty()) {
            SpawnTypes.clear();

            for (const auto &row : results) {
                addMonster(row["spm_race"].as<TYPE_OF_CHARACTER_ID>(),
                           row["spm_count"].as<int16_t>());
            }
        }
    } catch (std::exception &e) {
        Logger::error(LogFacility::Other) << "Exception in SpawnPoint::load: " << e.what() << Log::end;
        return false;
    }

    return true;
}
