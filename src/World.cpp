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

#include "World.hpp"

#include <dirent.h>
#include <boost/regex.hpp>
#include <algorithm>
#include <sys/types.h>

#include "make_unique.hpp"
#include "Logger.hpp"
#include "LongTimeAction.hpp"
#include "PlayerManager.hpp"
#include "Random.hpp"
#include "SchedulerTaskClasses.hpp"
#include "TableStructs.hpp"
#include "WaypointList.hpp"
#include "Config.hpp"
#include "Map.hpp"
#include "tuningConstants.hpp"

#include "data/Data.hpp"
#include "data/ScheduledScriptsTable.hpp"
#include "data/NPCTable.hpp"
#include "data/MonsterTable.hpp"
#include "data/TilesTable.hpp"
#include "data/SkillTable.hpp"
#include "data/WeaponObjectTable.hpp"

#include "script/LuaLogoutScript.hpp"
#include "script/LuaNPCScript.hpp"

#include "db/SelectQuery.hpp"
#include "db/Result.hpp"

#include "netinterface/BasicCommand.hpp"
#include "netinterface/NetInterface.hpp"
#include "netinterface/protocol/ServerCommands.hpp"

extern ScheduledScriptsTable *scheduledScripts;

//#define World_DEBUG

extern MonsterTable *MonsterDescriptions;
extern std::shared_ptr<LuaLogoutScript>logoutScript;

World *World::_self;

World *World::create(const std::string &dir, time_t starttime) {
    if (!(_self)) {
        _self = new World(dir,starttime);
        // init spawnlocations...
        _self->initRespawns();
        // initialise list of GM Commands
        _self->InitGMCommands();
        // initialise list of Player Commands
        _self->InitPlayerCommands();
        _self->monitoringClientList = std::make_unique<MonitoringClients>();
    }

    return _self;
}

World *World::get() throw(std::runtime_error) {
    if (!(_self)) {
        throw std::runtime_error("world was not created");
    }

    return _self;
}

World::World(const std::string &dir, time_t starttime) {

    nextXtoage = 0;

    lastTurnIGDay=getTime("day");

    usedAP = 0;
    timeStart = starttime*1000;

    currentScript = nullptr;

    directory = dir;
    scriptDir = dir + std::string(SCRIPTSDIR);

    timecount = 1;
    last_age = time(nullptr);
    ammount = 50;

    srand((unsigned) time(nullptr));

    unsigned int templi = starttime;
    char temparr[ 80 ];
    sprintf(temparr, "%u", templi);
}


struct editor_maptile {
    int32_t x;
    int32_t y;
    unsigned short fieldID;
    unsigned short int musicID;
};

int mapfilter(const struct dirent *d) {
    return (0 == strstr(d->d_name, ".tiles.txt"))?0:1;
}

bool World::load_maps() {
    // get all tiles files
    struct dirent **maplist;
    int numfiles = scandir((Config::instance().datadir() + "map/import/").c_str(), &maplist, mapfilter, alphasort);

    if (numfiles <= 0) {
        perror("Could not import maps");
        return false;
    }

    Logger::info(LogFacility::World) << "Importing " << numfiles << " maps." << Log::end;

    bool ok = true;

    // iterate over all map files...
    while (numfiles--) {

        // strip .tiles.txt from filename
        strstr(maplist[numfiles]->d_name, ".tiles.txt")[0] = '\0';
        Logger::debug(LogFacility::World) << "Importing: " << Config::instance().datadir() << "map/import/" << maplist[numfiles]->d_name << Log::end;

        ok &= load_from_editor(Config::instance().datadir() + "map/import/" + maplist[numfiles]->d_name);

    }

    return ok;

}

//! create a new world from editor files (new format)
bool World::load_from_editor(const std::string &filename) {
    // first try to open mapfile
    Logger::debug(LogFacility::World) << "try to Import map: " << filename << Log::end;
    std::ifstream maptilesfile((filename + ".tiles.txt").c_str());

    if (!maptilesfile.good()) {
        Logger::error(LogFacility::World) << "could not open file: " << filename << ".tiles.txt" << Log::end;
        return false;
    }

    int mapstartx = -1, mapstarty = -1;

    std::map<unsigned int, editor_maptile> maptiles;

    editor_maptile temp_tile;

    char dummy;

    int h_level, h_x, h_y, h_width, h_height, oldy;

    // load map file header information
    maptilesfile >> dummy;

    if (dummy == 'V') {
        maptilesfile >> dummy;
        int version;
        maptilesfile >> version;

        if (version != 2) {
            Logger::error(LogFacility::World) << "Invalid map format! Wrong version! Expected V2: " << filename << Log::end;
            return false;
        }
    } else {
        Logger::error(LogFacility::World) << "Invalid map format! No version: " << filename << Log::end;
        return false;
    }

    maptilesfile >> dummy;
    maptilesfile >> dummy;  // read 'L: '
    maptilesfile >> h_level;  //read int (level)
    maptilesfile >> dummy;
    maptilesfile >> dummy;  // read 'X: '
    maptilesfile >> h_x;  //read int (x coord)
    maptilesfile >> dummy;
    maptilesfile >> dummy;  // read 'Y: '
    maptilesfile >> h_y;  //read int (y coord)
    maptilesfile >> dummy;
    maptilesfile >> dummy;  // read 'W: '
    maptilesfile >> h_width;  //read int (width)
    maptilesfile >> dummy;
    maptilesfile >> dummy;  // read 'H: '
    maptilesfile >> h_height;  //read int (height)
    oldy = -1;

    Logger::debug(LogFacility::World) << "try to Import tiles: " << filename << Log::end;
    // load all tiles from the file
    maptilesfile >> temp_tile.x;  // read an int.

    while (maptilesfile.good()) {
        if (mapstartx == -1) {
            mapstartx = temp_tile.x;
        }

        temp_tile.x -= mapstartx;

        maptilesfile >> dummy;   // read a char (;)

        if (dummy != ';') {
            Logger::error(LogFacility::World) << "maptile file contains errors! : " << dummy << Log::end;
            return false;
        }

        maptilesfile >> temp_tile.y;    //read next int

        if (oldy != temp_tile.y) { //log if we have read one complete line of the map
            oldy = temp_tile.y;
        }

        if (mapstarty == -1) {
            mapstarty = temp_tile.y;
        }

        temp_tile.y -= mapstarty;

        maptilesfile >> dummy;          // read a char (;)

        if (dummy != ';') {
            Logger::error(LogFacility::World) << "maptile file contains errors! : " << dummy << Log::end;
            return false;
        }

        maptilesfile >> temp_tile.fieldID;      // read an int (tile-id)

        maptilesfile >> dummy;          // read a char (;)

        if (dummy != ';') {
            Logger::error(LogFacility::World) << "maptile file contains errors! : " << dummy << Log::end;
            return false;
        }

        maptilesfile >> temp_tile.musicID;      // read a short uint (music-id)

        // store the tile in our map
        maptiles[temp_tile.x<<16|temp_tile.y] = temp_tile;

        maptilesfile >> temp_tile.x;    // read next x (int); if there is none, while will end
    }

    // generate new map
    auto tempmap = std::make_shared<Map>(h_width, h_height);
    tempmap->Init(h_x, h_y, h_level);

    for (int x=0; x < h_width; ++x) {
        int index_start = x << 16;

        for (int y=0; y<h_height; ++y) {
            temp_tile = maptiles[index_start | y];
            Field *tempf;

            if (tempmap->GetPToCFieldAt(tempf, temp_tile.x+h_x, temp_tile.y+h_y)) {
                tempf->setTileId(temp_tile.fieldID);
                tempf->setMusicId(temp_tile.musicID);
                tempf->updateFlags();
            } else {
                Logger::error(LogFacility::World) << "could not get field for: " << x << " " << y << Log::end;
            }
        }
    }

    maptilesfile.close();
    maps.InsertMap(tempmap);

    // now try to load warpfields
    std::ifstream warpfile((filename + ".warps.txt").c_str());

    if (!warpfile.good()) {
        Logger::error(LogFacility::World) << "could not open file: " << filename << ".warps.txt" << Log::end;
        return true;    // warps are not crucial
    }

    position start, target;
    start.z = h_level;
    warpfile >> start.x;

    while (warpfile.good()) {
        warpfile >> dummy;

        if (dummy != ';') {
            Logger::error(LogFacility::World) << "warp file contains errors! : " << dummy << Log::end;
            return false;
        }

        warpfile >> start.y;
        warpfile >> dummy;

        if (dummy != ';') {
            Logger::error(LogFacility::World) << "warp file contains errors! : " << dummy << Log::end;
            return false;
        }

        warpfile >> target.x;
        warpfile >> dummy;

        if (dummy != ';') {
            Logger::error(LogFacility::World) << "warp file contains errors! : " << dummy << Log::end;
            return false;
        }

        warpfile >> target.y;
        warpfile >> dummy;

        if (dummy != ';') {
            Logger::error(LogFacility::World) << "warp file contains errors! : " << dummy << Log::end;
            return false;
        }

        warpfile >> target.z;

        start.x += h_x;
        start.y += h_y;
        GetField(start)->SetWarpField(target);

        warpfile >> start.x;
    }

    // next we try to load the items for the map
    std::ifstream mapitemsfile((filename + ".items.txt").c_str());

    if (!mapitemsfile.good()) {
        Logger::error(LogFacility::World) << "could not open file: " << filename << ".items.txt" << Log::end;
        return true;    // items are not crucial
    }

    int x,y;
    Item it;
    Item::id_type itemId;
    Item::quality_type itemQuality;
    oldy = -1;
    Logger::debug(LogFacility::World) << "try to import items: " << filename << Log::end;
    mapitemsfile >> x;

    while (mapitemsfile.good()) {
        it.reset();
        it.makePermanent();
        it.setNumber(1);
        x -= mapstartx;
        x += h_x;
        mapitemsfile >> dummy;

        if (dummy != ';') {
            Logger::error(LogFacility::World) << "mapitem file contains errors! : " << dummy << Log::end;
            return false;
        }

        mapitemsfile >> y;

        if (oldy != y) { //log if we have read one complete line of the map
            oldy = y;
        }

        y -= mapstarty;
        y += h_y;
        mapitemsfile >> dummy;

        if (dummy != ';') {
            Logger::error(LogFacility::World) << "mapitem file contains errors! : " << dummy << Log::end;
            return false;
        }

        mapitemsfile >> itemId;
        it.setId(itemId);

        mapitemsfile >> dummy;

        if (dummy != ';') {
            Logger::error(LogFacility::World) << "mapitem file contains errors! : " << dummy << Log::end;
            return false;
        }

        mapitemsfile >> itemQuality;
        it.setQuality(itemQuality);

        if (mapitemsfile.good()) {
            if (mapitemsfile.get() == ';') {
                std::string dataSequence;
                std::getline(mapitemsfile, dataSequence);
                std::string key, value;
                bool isKey = true;

                for (size_t i = 0; i < dataSequence.length(); ++i) {
                    char c = dataSequence[i];

                    switch (c) {
                    case ';':
                        it.setData(key, value);
                        key = "";
                        value = "";
                        isKey = true;
                        break;

                    case '=':
                        isKey = false;
                        break;

                    case '\\':
                        ++i;

                        if (i == dataSequence.length()) {
                            return false;
                        }

                        c = dataSequence[i];

                    default:

                        if (isKey) {
                            key = key + c;
                        } else {
                            value = value + c;
                        }
                    }
                }

                it.setData(key, value);
            } else {
                mapitemsfile.unget();
            }
        }

        // store the item in our map
        g_item = it;
        g_cont = nullptr;

        if (!putItemAlwaysOnMap(nullptr, position(x, y, h_level))) {
            Logger::info(LogFacility::World) << "could not put item" << Log::end;
        }

        mapitemsfile >> x;
    }

    mapitemsfile.close();
    Logger::debug(LogFacility::World) << "Import map: " << filename << " was successful!" << Log::end;

    return true;
}

World::~World() {
}


void World::turntheworld() {
    ftime(&now);
    unsigned long timeNow = now.time*1000 + now.millitm;

    int thisIGDay = getTime("day");

    if (lastTurnIGDay!=thisIGDay) {
        sendIGTimeToAllPlayers();
        Logger::debug(LogFacility::World) << "lastTurnIGDay=" << lastTurnIGDay << " thisIGDay= " << thisIGDay << Log::end;
        lastTurnIGDay=thisIGDay;
    }

    ap = timeNow/MIN_AP_UPDATE - timeStart/MIN_AP_UPDATE - usedAP;

    if (ap > 0) {
        usedAP += ap;

        checkPlayers();
        checkMonsters();
        checkNPC();

        if (monitoringclienttimer.Next()) {
            monitoringClientList->CheckClients();
        }

        if (schedulertimer.next()) {
            scheduler->NextCycle();
        }

        if (ScriptTimer.next()) {
            scheduledScripts->nextCycle();
        }
    }

    DoAge();
}



void World::checkPlayers() {
    time_t tempkeepalive;
    time(&tempkeepalive);

    std::vector<Player *> lostPlayers;

    Players.for_each([tempkeepalive, &lostPlayers, this](Player *playerPointer) {
        Player &player = *playerPointer;

        if (player.Connection->online) {
            int temptime = tempkeepalive - player.lastkeepalive;

            if (((temptime >= 0) && (temptime <= CLIENT_TIMEOUT))) {
                player.increaseActionPoints(ap);
                player.increaseFightPoints(ap);
                player.workoutCommands();
                player.ltAction->checkAction();
                player.effects.checkEffects();
            }
            // User timed out.
            else {
                Logger::info(LogFacility::World) << player << " timed out " << temptime << Log::end;
                ServerCommandPointer cmd = std::make_shared<LogOutTC>(UNSTABLECONNECTION);
                player.Connection->shutdownSend(cmd);
            }
        } else {
            const position &pos = player.getPosition();

            Logger::info(LogFacility::World) << player << " is offline" << Log::end;
            Field *tempf;

            if (GetPToCFieldAt(tempf, pos)) {
                tempf->SetPlayerOnField(false);
            }

            Logger::info(LogFacility::Player) << "logout of " << player << Log::end;

            logoutScript->onLogout(playerPointer);

            PlayerManager::get().getLogOutPlayers().non_block_push_back(playerPointer);
            sendRemoveCharToVisiblePlayers(player.getId(), pos);
            lostPlayers.push_back(playerPointer);
        }
    });

    for (const auto &player : lostPlayers) {
        Players.erase(player->getId());
    }
}

void World::invalidatePlayerDialogs() {
    Players.for_each(&Player::invalidateDialogs);
}

// init the respawn locations... for now still hardcoded...
bool World::initRespawns() {
    Monsters.for_each([](Monster *monster) {
        monster->remove();
        monster->setSpawn(nullptr);
    });

    SpawnList.clear();

    // read spawnpoints from db

    try {
        Database::SelectQuery query;
        query.addColumn("spawnpoint", "spp_id");
        query.addColumn("spawnpoint", "spp_x");
        query.addColumn("spawnpoint", "spp_y");
        query.addColumn("spawnpoint", "spp_z");
        query.addColumn("spawnpoint", "spp_range");
        query.addColumn("spawnpoint", "spp_spawnrange");
        query.addColumn("spawnpoint", "spp_minspawntime");
        query.addColumn("spawnpoint", "spp_maxspawntime");
        query.addColumn("spawnpoint", "spp_spawnall");
        query.addServerTable("spawnpoint");

        Database::Result results = query.execute();

        if (!results.empty()) {

            for (const auto &row : results) {
                const uint32_t spawnId = row["spp_id"].as<uint32_t>();
                const position pos(row["spp_x"].as<int16_t>(),
                                   row["spp_y"].as<int16_t>(),
                                   row["spp_z"].as<int16_t>());
                SpawnPoint newSpawn(pos,
                                    row["spp_range"].as<int>(),
                                    row["spp_spawnrange"].as<uint16_t>(),
                                    row["spp_minspawntime"].as<uint16_t>(),
                                    row["spp_maxspawntime"].as<uint16_t>(),
                                    row["spp_spawnall"].as<bool>());
                Logger::debug(LogFacility::World) << "load spawnpoint " << spawnId << ":" << Log::end;
                newSpawn.load(spawnId);
                SpawnList.push_back(newSpawn);
                Logger::debug(LogFacility::World) << "added spawnpoint " << pos << Log::end;
            }

        } else {
            return false;
        }

        return true; // everything went well
    } catch (std::exception &e) {
        Logger::error(LogFacility::World) << "got exception in load SpawnPoints: " << e.what() << Log::end;
        return false;
    }

}

void World::checkMonsters() {

    // respawn ?
    if (monstertimer.next()) {
        if (isSpawnEnabled()) {
            for (auto &spawn : SpawnList) {
                spawn.spawn();
            }
        } else {
            Logger::info(LogFacility::World) << "World::checkMonsters() spawning disabled!" << Log::end;
        }
    }

    if (ap > 1) {
        --ap;
    }

    std::vector<Monster *> deadMonsters;

    Monsters.for_each([this, &deadMonsters](Monster *monsterPointer) {
        Monster &monster = *monsterPointer;

        if (monster.isAlive()) {
            monster.increaseActionPoints(ap);
            monster.increaseFightPoints(ap);
            monster.effects.checkEffects();

            MonsterStruct monStruct;
            bool foundMonster = MonsterDescriptions->find(monster.getMonsterType(), monStruct) ;

            if (monster.canAct()) {
                if (!monster.getOnRoute()) {
                    //set lastTargetSeen to false if we reach the position where the target was seen the last time
                    if (monster.getPosition() == monster.lastTargetPosition) {
                        monster.lastTargetSeen = false;
                    }

                    // enough AP
                    //searh for all players which can be attacked from the monster directly

                    //get attackrange of the weapon
                    Item itl = monster.GetItemAt(LEFT_TOOL);
                    Item itr = monster.GetItemAt(RIGHT_TOOL);

                    uint16_t range=1;

                    if (Data::WeaponItems.exists(itr.getId())) {
                        range = Data::WeaponItems[itr.getId()].Range;
                    } else if (Data::WeaponItems.exists(itl.getId())) {
                        range = Data::WeaponItems[itl.getId()].Range;;
                    }

                    //===============================================
                    const auto temp = Players.findAllAliveCharactersInRangeOf(monster.getPosition(), range);
                    bool has_attacked=false;
                    //If we have found players which can be attacked directly and the monster can attack
                    Player *foundP = nullptr;

                    if ((!temp.empty()) && monster.canAttack()) {
                        //angreifen
                        //search for the target via script or the player with the lowest hp
                        if (!monStruct.script || !monStruct.script->setTarget(monsterPointer, temp, foundP)) {
                            findPlayerWithLowestHP(temp, foundP);
                        }

                        if (foundP) {
                            //let the monster attack the player with the lowest hp->assigned this player as target
                            monster.enemyid = foundP->getId();
                            monster.enemytype = Character::player;
                            monster.lastTargetPosition = foundP->getPosition();
                            monster.lastTargetSeen = true;

                            if (foundMonster) {
                                //check if we have a pointer to a script
                                if (monStruct.script) {
                                    //Wenn Scriptaufruf erfolgreich den aktuellen schleifenablauf abbrechen.
                                    if (monStruct.script->enemyNear(monsterPointer, foundP)) {
                                        return; //Schleife fr dieses Monster abbrechen. Da es schon etwas diesne Schleifendurchlauf getan hat.
                                    }
                                }
                            } else {
                                Logger::error(LogFacility::Script) << "cant find a monster id for checking the script!" << Log::end;
                            }

                            //attack the player which we have found
                            monster.turn(foundP->getPosition());

                            if (monster.canFight()) {    // enough FP to fight?
                                has_attacked = characterAttacks(monsterPointer);
                            } else {
                                has_attacked = true;
                            }
                        }
                    }

                    if (!has_attacked) { //bewegen
                        const auto temp = Players.findAllAliveCharactersInRangeOf(monster.getPosition(), MONSTERVIEWRANGE);

                        bool makeRandomStep=true;

                        if ((!temp.empty()) && (monster.canAttack())) {
                            Player *foundP2 = nullptr;

                            //search for the target via script or the player with the lowest hp
                            if (!monStruct.script || !monStruct.script->setTarget(monsterPointer, temp, foundP2)) {
                                findPlayerWithLowestHP(temp, foundP2);
                            }

                            if (foundP2) {  // if the script returned a valid character...
                                monster.lastTargetSeen = true;
                                monster.lastTargetPosition = foundP2->getPosition();

                                //Call enemyNear Script when enemy found
                                if (foundMonster) {
                                    if (monStruct.script) {
                                        if (monStruct.script->enemyOnSight(monsterPointer, foundP2)) {
                                            return; //abort all other walking actions because the script has returned TRUE
                                        }
                                    }

                                    makeRandomStep=false;
                                    monster.performStep(foundP2->getPosition());
                                } else {
                                    Logger::notice(LogFacility::Script) << "cant find the monster id for calling a script!" << Log::end;
                                }

                            }
                        } else if (monster.lastTargetSeen) {
                            makeRandomStep=false;
                            monster.performStep(monster.lastTargetPosition);
                        }

                        if (makeRandomStep) {
                            // No player in range or pig/sheep OR we didn't find anything in getTarget...
                            int tempr = Random::uniform(1, 25);

                            MonsterStruct monsterdef;

                            bool hasDefinition = MonsterDescriptions->find(monster.getMonsterType() , monsterdef);

                            if (!hasDefinition) {
                                Logger::error(LogFacility::World) << "Data for Healing not Found for monsterrace: " << monster.getMonsterType() << Log::end;
                            }

                            if (tempr <= 5 && hasDefinition && monsterdef.canselfheal) {
                                monster.heal();
                            } else {
                                SpawnPoint *spawn = monster.getSpawn();

                                direction dir = (direction)Random::uniform(0,7);

                                if (spawn) {
                                    int yoffs = monster.getPosition().y - spawn->get_y();
                                    int xoffs = monster.getPosition().x - spawn->get_x();

                                    if (abs(xoffs) > spawn->getRange() || abs(yoffs) > spawn->getRange()) {
                                        // monster out of spawn range, remove it from spawn
                                        monster.setSpawn(nullptr);
                                        unsigned int type = monster.getMonsterType();
                                        spawn->dead(type);
                                    }

                                    position newpos = monster.getPosition();
                                    newpos.move(dir);
                                    yoffs = spawn->get_y() - newpos.y;
                                    xoffs = spawn->get_x() - newpos.x;

                                    // if walking out of range, mirroring dir. at spawn area border lets the char stay in range with L_inf metric
                                    if (abs(xoffs) > spawn->getRange()) {
                                        switch (dir) {
                                        case dir_northeast:
                                            dir = dir_northwest;
                                            break;

                                        case dir_east:
                                            dir = dir_west;
                                            break;

                                        case dir_southeast:
                                            dir = dir_southwest;
                                            break;

                                        case dir_southwest:
                                            dir = dir_southeast;
                                            break;

                                        case dir_west:
                                            dir = dir_east;
                                            break;

                                        case dir_northwest:
                                            dir = dir_northeast;
                                            break;

                                        default:
                                            break;
                                        }
                                    }

                                    if (abs(yoffs) > spawn->getRange()) {
                                        switch (dir) {
                                        case dir_north:
                                            dir = dir_south;
                                            break;

                                        case dir_northeast:
                                            dir = dir_southeast;
                                            break;

                                        case dir_southeast:
                                            dir = dir_northeast;
                                            break;

                                        case dir_south:
                                            dir = dir_north;
                                            break;

                                        case dir_southwest:
                                            dir = dir_northwest;
                                            break;

                                        case dir_northwest:
                                            dir = dir_southwest;
                                            break;

                                        default:
                                            break;
                                        }
                                    }
                                }

                                monster.move(dir);

                                // movementrate below normal if noone is near
                                monster.increaseActionPoints(-20);
                            }
                        }
                    }//angreifen/bewegen
                } else { //Character is on route
                    //get attackrange of the weapon
                    Item itl = monster.GetItemAt(LEFT_TOOL);
                    Item itr = monster.GetItemAt(RIGHT_TOOL);

                    uint16_t range=1;

                    if (Data::WeaponItems.exists(itr.getId())) {
                        range = Data::WeaponItems[itr.getId()].Range;
                    } else if (Data::WeaponItems.exists(itl.getId())) {
                        range = Data::WeaponItems[itl.getId()].Range;;
                    }

                    //===============================================
                    const auto temp = Players.findAllAliveCharactersInRangeOf(monster.getPosition(), range);

                    //If we have found players which can be attacked directly and the monster can attack
                    if (!temp.empty()) {
                        //angreifen
                        Player *foundP;

                        //search for the player with the lowes hp
                        if (findPlayerWithLowestHP(temp, foundP)) {
                            if (foundMonster && monStruct.script) {
                                monStruct.script->enemyNear(monsterPointer, foundP);
                            } else {
                                Logger::error(LogFacility::World) << "cant find a monster id for checking the script!" << Log::end;
                            }

                        }
                    }

                    //check if there is a player on sight
                    const auto temp2 = Players.findAllAliveCharactersInRangeOf(monster.getPosition(), MONSTERVIEWRANGE);

                    if (!temp2.empty()) {
                        Player *foundP = nullptr;

                        //search for the target via script or the player with the lowest hp
                        if (!monStruct.script || !monStruct.script->setTarget(monsterPointer, temp2, foundP)) {
                            findPlayerWithLowestHP(temp2, foundP);
                        }

                        if (foundP) {
                            //Call enemyNear Script when enemy found
                            if (foundMonster && monStruct.script) {
                                monStruct.script->enemyOnSight(monsterPointer, foundP);
                            }
                        }
                    }

                    if (!monster.waypoints.makeMove()) {
                        monster.setOnRoute(false);

                        if (foundMonster && monStruct.script) {
                            monStruct.script->abortRoute(monsterPointer);
                        } else {
                            Logger::notice(LogFacility::Script) << "cant find the monster id for calling a script!" << Log::end;
                        }
                    }
                }
            }
        } else {
            deadMonsters.push_back(monsterPointer);
        }
    });

    for (const auto &monster : deadMonsters) {
        killMonster(monster->getId());
    }

    for (auto &monster : newMonsters) {
        Monsters.insert(monster);
        MonsterStruct monStruct;
        bool foundMonster = MonsterDescriptions->find(monster->getMonsterType(), monStruct) ;

        if (foundMonster && monStruct.script) {
            monStruct.script->onSpawn(monster);
        }

    }

    newMonsters.clear();
}



void World::checkNPC() {
    deleteAllLostNPC();

    Npc.for_each([this](NPC* npc) {

        if (npc->isAlive()) {
            npc->increaseActionPoints(ap);
            npc->effects.checkEffects();
            std::shared_ptr<LuaNPCScript> npcScript = npc->getScript();

            if (npc->canAct() && npcScript) {
                npcScript->nextCycle();

                if (npc->getOnRoute() && !npc->waypoints.makeMove()) {
                    npc->setOnRoute(false);
                    npcScript->abortRoute();
                }
            }
        } else {
            npc->increaseAttrib("hitpoints", MAXHPS);
            sendSpinToAllVisiblePlayers(npc);
        }
    });
}


void World::workout_CommandBuffer(Player *&cp) {
}


// Init method for NPC's
void World::initNPC() {
    Npc.for_each([this](NPC *npc) {
        Field *tempf;

        if (GetPToCFieldAt(tempf, npc->getPosition())) {
            tempf->removeChar();
        }

        sendRemoveCharToVisiblePlayers(npc->getId(), npc->getPosition());
        delete npc;
    });

    Npc.clear();
    NPCTable NPCTbl;
}

void World::initScheduler() {
    scheduler = std::make_unique<Scheduler>();
    auto globalPlLearning = std::make_unique<SGlobalPlayerLearnrate>(scheduler->GetCurrentCycle()+5);
    scheduler->AddTask(std::move(globalPlLearning));
    auto globalMonLearning = std::make_unique<SGlobalMonsterLearnrate>(scheduler->GetCurrentCycle()+10);
    scheduler->AddTask(std::move(globalMonLearning));
}

bool World::executeUserCommand(Player *user, const std::string &input, const CommandMap &commands) {
    bool found = false;

    static const boost::regex pattern("^!([^ ]+) ?(.*)?$");
    boost::smatch match;

    if (boost::regex_match(input, match, pattern)) {
        auto it = commands.find(match[1].str());

        if (it != commands.end()) {
            (it->second)(this, user, match[2].str());
            found = true;
        }
    }

    return found;
}

