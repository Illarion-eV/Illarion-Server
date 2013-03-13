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
#include <regex.h>
#include <algorithm>
#include <sys/types.h>

#include "data/MonsterTable.hpp"
#include "data/TilesTable.hpp"
#include "data/SkillTable.hpp"
#include "data/WeaponObjectTable.hpp"

#include "script/LuaLogoutScript.hpp"

#include "db/SelectQuery.hpp"
#include "db/Result.hpp"

#include "netinterface/BasicCommand.hpp"
#include "netinterface/NetInterface.hpp"
#include "netinterface/protocol/ServerCommands.hpp"

#include "Logger.hpp"
#include "LongTimeAction.hpp"
#include "PlayerManager.hpp"
#include "Random.hpp"
#include "SchedulerTaskClasses.hpp"
#include "TableStructs.hpp"
#include "WaypointList.hpp"
#include "Config.hpp"

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
        _self->monitoringClientList = new MonitoringClients(_self);

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

    currentScript = NULL;

    directory = dir;
    scriptDir = dir + std::string(SCRIPTSDIR);

    fieldtimer[ 0 ] = new Timer(5);       // 5 s
    fieldtimer[ 1 ] = new Timer(5);       // 5 s
    fieldtimer[ 2 ] = new Timer(5);       // 5 s

    monstertimer = new Timer(10);       // 60 s

    schedulertimer = new Timer(1);      // 1 s

    ScriptTimer = new Timer(1);   //1 s

    npctimer = new MilTimer(2000);         // 2 s

    monitoringclienttimer = new MilTimer(250);

    timecount = 1;
    last_age = time(NULL);
    ammount = 50;

    srand((unsigned) time(NULL));

    // Bewegung nach Norden
    moveSteps[ 0 ][ 0 ] = 0;      // x - Koordinate
    moveSteps[ 0 ][ 1 ] = -1;     // y - Koordinate
    moveSteps[ 0 ][ 2 ] = 0;      // z - Koordinate

    // NO
    moveSteps[ 1 ][ 0 ] = 1;
    moveSteps[ 1 ][ 1 ] = -1;
    moveSteps[ 1 ][ 2 ] = 0;

    // O
    moveSteps[ 2 ][ 0 ] = 1;
    moveSteps[ 2 ][ 1 ] = 0;
    moveSteps[ 2 ][ 2 ] = 0;

    // SO
    moveSteps[ 3 ][ 0 ] = 1;
    moveSteps[ 3 ][ 1 ] = 1;
    moveSteps[ 3 ][ 2 ] = 0;

    // S
    moveSteps[ 4 ][ 0 ] = 0;
    moveSteps[ 4 ][ 1 ] = 1;
    moveSteps[ 4 ][ 2 ] = 0;

    // SW
    moveSteps[ 5 ][ 0 ] = -1;
    moveSteps[ 5 ][ 1 ] = 1;
    moveSteps[ 5 ][ 2 ] = 0;

    // W
    moveSteps[ 6 ][ 0 ] = -1;
    moveSteps[ 6 ][ 1 ] = 0;
    moveSteps[ 6 ][ 2 ] = 0;

    // NW
    moveSteps[ 7 ][ 0 ] = -1;
    moveSteps[ 7 ][ 1 ] = -1;
    moveSteps[ 7 ][ 2 ] = 0;

    // up
    moveSteps[ 8 ][ 0 ] = 0;
    moveSteps[ 8 ][ 1 ] = 0;
    moveSteps[ 8 ][ 2 ] = 1;

    // down
    moveSteps[ 9 ][ 0 ] = 0;
    moveSteps[ 9 ][ 1 ] = 0;
    moveSteps[ 9 ][ 2 ] = -1;

    // no move
    moveSteps[ 10 ][ 0 ] = 0;
    moveSteps[ 10 ][ 1 ] = 0;
    moveSteps[ 10 ][ 2 ] = 0;

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

    bool ok = true;

    // iterate over all map files...
    while (numfiles--) {

        // strip .tiles.txt from filename
        strstr(maplist[numfiles]->d_name, ".tiles.txt")[0] = '\0';
        Logger::info(LogFacility::World) << "importing: " << Config::instance().datadir() << "map/import/" << maplist[numfiles]->d_name << Log::end;

        ok &= load_from_editor(Config::instance().datadir() + "map/import/" + maplist[numfiles]->d_name);

    }

    return ok;

}

//! create a new world from editor files (new format)
bool World::load_from_editor(const std::string &filename) {
    // first try to open mapfile
    Logger::info(LogFacility::World) << "try to Import map: " << filename << Log::end;
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

    Logger::info(LogFacility::World) << "try to Import tiles: " << filename << Log::end;
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
    WorldMap::map_t tempmap(new Map(h_width, h_height));
    tempmap->Init(h_x, h_y, h_level);

    Field *tempf;
    int index_start;

    for (int x=0; x<h_width; ++x) {
        index_start = x << 16;

        for (int y=0; y<h_height; ++y) {
            temp_tile = maptiles[index_start | y];

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
    Logger::info(LogFacility::World) << "try to import items: " << filename << Log::end;
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
        g_cont = NULL;

        if (!putItemAlwaysOnMap(NULL, x, y, h_level)) {
            Logger::info(LogFacility::World) << "could not put item" << Log::end;
        }

        mapitemsfile >> x;
    }

    mapitemsfile.close();
    Logger::info(LogFacility::World) << "Import map: " << filename << " was successful!" << Log::end;

    return true;
}

World::~World() {
    delete monstertimer;
    monstertimer = NULL;
    delete npctimer;
    npctimer = NULL;
    delete fieldtimer[ 0 ];
    fieldtimer[ 0 ] = NULL;
    delete fieldtimer[ 1 ];
    fieldtimer[ 1 ] = NULL;
    delete fieldtimer[ 2 ];
    fieldtimer[ 2 ] = NULL;
    delete monitoringclienttimer;
    monitoringclienttimer = NULL;
    delete monitoringClientList;
    monitoringClientList = NULL;

    delete scheduler;
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

        if (monitoringclienttimer->Next()) {
            monitoringClientList->CheckClients();
        }

        //N�hsten Scheduler Cycle einleiten nur zu Testzwecke
        //Der Scheduler soll in einen eigenen Thread laufen.
        if (schedulertimer->next()) {
            scheduler->NextCycle();
        }

        if (ScriptTimer->next()) {
            scheduledScripts->nextCycle();
        }
    }

    DoAge();
}



void World::checkPlayers() {
    time_t tempkeepalive;
    time(&tempkeepalive);
    int temptime;

    auto playerIterator = Players.begin();

    while (playerIterator < Players.end()) {
        if ((*playerIterator)->Connection->online) {
            temptime = tempkeepalive - (*playerIterator)->lastkeepalive;

            if (((temptime >= 0) && (temptime <= CLIENT_TIMEOUT))) {
                (*playerIterator)->actionPoints += ap;
                (*playerIterator)->fightPoints += ap;

                if ((*playerIterator)->actionPoints > P_MAX_AP) {
                    (*playerIterator)->actionPoints = P_MAX_AP;
                }

                if ((*playerIterator)->fightPoints > P_MAX_FP) {
                    (*playerIterator)->fightPoints = P_MAX_FP;
                }

                if ((*playerIterator)->actionPoints >= P_MIN_AP) {
                    (*playerIterator)->workoutCommands();
                }

                (*playerIterator)->ltAction->checkAction();
                (*playerIterator)->effects.checkEffects();
            }
            // User timed out.
            else {
                Logger::info(LogFacility::World) << (*playerIterator)->to_string() << " timed out " << temptime << Log::end;
                ServerCommandPointer cmd(new LogOutTC(UNSTABLECONNECTION));
                (*playerIterator)->Connection->shutdownSend(cmd);
            }

            ++playerIterator;
        } else {
            auto temp_id = (*playerIterator)->getId();
            position temp_pos = (*playerIterator)->pos;

            Logger::info(LogFacility::World) << (*playerIterator)->to_string() << " is offline" << Log::end;
            Field *tempf;

            if (GetPToCFieldAt(tempf, (*playerIterator)->pos.x, (*playerIterator)->pos.y, (*playerIterator)->pos.z)) {
                tempf->SetPlayerOnField(false);
            }

            Logger::info(LogFacility::Player) << "logout of " << (*playerIterator)->to_string() << Log::end;

            logoutScript->onLogout(*playerIterator);

            PlayerManager::get()->getLogOutPlayers().non_block_push_back(*playerIterator);
            playerIterator = Players.erase(playerIterator);
            sendRemoveCharToVisiblePlayers(temp_id, temp_pos);
        }
    }
}

void World::invalidatePlayerDialogs() {
    for (auto it = Players.begin(); it != Players.end(); ++it) {
        (*it)->invalidateDialogs();
    }
}

// init the respawn locations... for now still hardcoded...
bool World::initRespawns() {
    // if we have monsters, we need to delete their spawnpoints...
    for (MONSTERVECTOR::iterator monsterIterator = Monsters.begin(); monsterIterator != Monsters.end(); ++monsterIterator) {
        // if the spawn is set to NULL it is regarded as no spawnpoint.
        (*monsterIterator)->remove();
        (*monsterIterator)->setSpawn(NULL);
    }

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
            uint32_t spawnId;

            for (Database::ResultConstIterator itr = results.begin();
                 itr != results.end(); ++itr) {
                spawnId = (*itr)["spp_id"].as<uint32_t>();
                position the_pos((*itr)["spp_x"].as<int32_t>(),
                                 (*itr)["spp_y"].as<int32_t>(),
                                 (*itr)["spp_z"].as<int32_t>());
                SpawnPoint newSpawn(the_pos,
                                    (*itr)["spp_range"].as<int>(),
                                    (*itr)["spp_spawnrange"].as<uint16_t>(),
                                    (*itr)["spp_minspawntime"].as<uint16_t>(),
                                    (*itr)["spp_maxspawntime"].as<uint16_t>(),
                                    (*itr)["spp_spawnall"].as<bool>());
                Logger::debug(LogFacility::World) << "load spawnpoint " << spawnId << ":" << Log::end;
                newSpawn.load(spawnId);
                SpawnList.push_back(newSpawn);
                Logger::debug(LogFacility::World) << "added spawnpoint " << the_pos.toString() << Log::end;
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
    if (monstertimer->next()) {
        std::list<SpawnPoint>::iterator it;

        if (isSpawnEnabled()) {
            for (it = SpawnList.begin(); it != SpawnList.end(); ++it) {
                it->spawn();
            }
        } else {
            Logger::info(LogFacility::World) << "World::checkMonsters() spawning disabled!" << Log::end;
        }
    }

    std::vector < Player * > temp;
    auto monsterIterator = Monsters.begin();

    if (ap > 1) {
        --ap;
    }

    while (monsterIterator < Monsters.end()) {
        Monster &monster = **monsterIterator;

        if (monster.IsAlive()) {
            // monster alive
            monster.actionPoints += ap;
            monster.fightPoints += ap;

            if (monster.actionPoints > NP_MAX_AP) {         // too many AP
                monster.actionPoints = NP_MAX_AP;
            }

            if (monster.fightPoints > NP_MAX_FP) {         // too many FP
                monster.fightPoints = NP_MAX_FP;
            }

            monster.effects.checkEffects();

            MonsterStruct monStruct;
            bool foundMonster = MonsterDescriptions->find(monster.getType(), monStruct) ;

            if (monster.actionPoints >= NP_MIN_AP) {
                if (!monster.getOnRoute()) {
                    //set lastTargetSeen to false if we reach the position where the target was seen the last time
                    if (monster.pos == monster.lastTargetPosition) {
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
                    temp = Players.findAllAliveCharactersInRangeOf(monster.pos.x, monster.pos.y, monster.pos.z, range);
                    bool has_attacked=false;
                    //If we have found players which can be attacked directly and the monster can attack
                    Player *foundP = 0;

                    if ((!temp.empty()) && monster.canAttack()) {
                        //angreifen
                        //search for the target via script or the player with the lowest hp
                        if (!monStruct.script || !monStruct.script->setTarget(*monsterIterator, temp, foundP)) {
                            findPlayerWithLowestHP(temp, foundP);
                        }

                        if (foundP) {
                            //let the monster attack the player with the lowest hp->assigned this player as target
                            monster.enemyid = foundP->getId();
                            monster.enemytype = Character::player;
                            monster.lastTargetPosition = foundP->pos;
                            monster.lastTargetSeen = true;

                            if (foundMonster) {
                                //check if we have a pointer to a script
                                if (monStruct.script) {
                                    //Wenn Scriptaufruf erfolgreich den aktuellen schleifenablauf abbrechen.
                                    if (monStruct.script->enemyNear(*monsterIterator, foundP)) {
                                        continue; //Schleife fr dieses Monster abbrechen. Da es schon etwas diesne Schleifendurchlauf getan hat.
                                    }
                                }
                            } else {
                                Logger::error(LogFacility::Script) << "cant find a monster id for checking the script!" << Log::end;
                            }

                            //attack the player which we have found
                            monster.turn(foundP->pos);

                            if (monster.fightPoints >= NP_MIN_FP) {    // enough FP to fight?
                                has_attacked = characterAttacks(*monsterIterator);
                            } else {
                                has_attacked = true;
                            }
                        }
                    }

                    if (!has_attacked) { //bewegen
                        temp = Players.findAllAliveCharactersInRangeOf(monster.pos.x, monster.pos.y, monster.pos.z, 15);

                        bool makeRandomStep=true;

                        if ((!temp.empty()) && (monster.canAttack())) {
                            Player *foundP2 = 0;

                            //search for the target via script or the player with the lowest hp
                            if (!monStruct.script || !monStruct.script->setTarget(*monsterIterator, temp, foundP2)) {
                                findPlayerWithLowestHP(temp, foundP2);
                            }

                            if (foundP2) {  // if the script returned a valid character...
                                monster.lastTargetSeen = true;
                                monster.lastTargetPosition = foundP2->pos;

                                //Call enemyNear Script when enemy found
                                if (foundMonster) {
                                    if (monStruct.script) {
                                        //Wenn Scriptaufruf erfolgreich den aktuellen schleifenablauf abbrechen.
                                        if (monStruct.script->enemyOnSight(*monsterIterator, foundP2)) {
                                            continue; //abort all other walking actions because the script has returned TRUE
                                        }
                                    }

                                    makeRandomStep=false;
                                    monster.performStep(foundP2->pos);
                                } else {
                                    Logger::info(LogFacility::Script) << "cant find the monster id for calling a script!" << Log::end;
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

                            bool hasDefinition = MonsterDescriptions->find(monster.getType() , monsterdef);

                            if (!hasDefinition) {
                                Logger::error(LogFacility::World) << "Data for Healing not Found for monsterrace: " << monster.getType() << Log::end;
                            }

                            if (tempr <= 5 && hasDefinition && monsterdef.canselfheal) {
                                monster.heal();
                            } else {
                                SpawnPoint *spawn = monster.getSpawn();

                                direction dir = (direction)Random::uniform(0,7);

                                if (spawn) {
                                    int yoffs = monster.pos.y - spawn->get_y();
                                    int xoffs = monster.pos.x - spawn->get_x();

                                    if (abs(xoffs) > spawn->getRange() || abs(yoffs) > spawn->getRange()) {
                                        // monster out of spawn range, remove it from spawn
                                        monster.setSpawn(NULL);
                                        unsigned int type = monster.getType();
                                        spawn->dead(type);
                                    }

                                    position newpos = monster.pos;
                                    newpos.x += moveSteps[ dir ][ 0 ];
                                    newpos.y += moveSteps[ dir ][ 1 ];
                                    newpos.z += moveSteps[ dir ][ 2 ];
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
                                monster.actionPoints -= 20;
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
                    temp = Players.findAllAliveCharactersInRangeOf(monster.pos.x, monster.pos.y, monster.pos.z, range);

                    //If we have found players which can be attacked directly and the monster can attack
                    if (!temp.empty()) {
                        //angreifen
                        Player *foundP;

                        //search for the player with the lowes hp
                        if (findPlayerWithLowestHP(temp, foundP)) {
                            if (foundMonster && monStruct.script) {
                                monStruct.script->enemyNear(*monsterIterator, foundP);
                            } else {
                                Logger::error(LogFacility::World) << "cant find a monster id for checking the script!" << Log::end;
                            }

                        }
                    }

                    //check if there is a player on sight
                    temp = Players.findAllAliveCharactersInRangeOf(monster.pos.x, monster.pos.y, monster.pos.z, 15);

                    if (!temp.empty()) {
                        Player *foundP;

                        if (findPlayerWithLowestHP(temp, foundP)) {
                            //Call enemyNear Script when enemy found
                            if (foundMonster && monStruct.script) {
                                monStruct.script->enemyOnSight(*monsterIterator, foundP);
                            }

                            //else Logger::writeMessage("Monster","cant find the monster id for calling a script!");
                        }
                    }

                    if (!monster.waypoints.makeMove()) {
                        monster.setOnRoute(false);

                        if (foundMonster && monStruct.script) {
                            monStruct.script->abortRoute(*monsterIterator);
                        } else {
                            Logger::info(LogFacility::Script) << "cant find the monster id for calling a script!" << Log::end;
                        }
                    }
                }
            } // ausreichend actionpoints

            monsterIterator++;
        } //alive ?
        else {
            // tot
            MONSTERVECTOR::iterator newIt;
            killMonster(monsterIterator, newIt);
            monsterIterator = newIt;
        } //alive ?
    }//alle Monster abgearbeitet

    //Adding all new monsters to monsterlist und calling the onSpawn Script entry
    for (MONSTERVECTOR::iterator newMonIt = newMonsters.begin(); newMonIt != newMonsters.end(); ++newMonIt) {
        Monsters.push_back((*newMonIt));
        MonsterStruct monStruct;
        bool foundMonster = MonsterDescriptions->find((*newMonIt)->getType(), monStruct) ;

        if (foundMonster && monStruct.script) {
            monStruct.script->onSpawn((*newMonIt));
        }

    }

    //clear new monsters so they can be added
    newMonsters.clear();
}



void World::checkNPC() {
    deleteAllLostNPC();

    NPCVECTOR::iterator npcIterator = Npc.begin();

    while (npcIterator < Npc.end()) {

        if ((*npcIterator)->IsAlive()) {
            (*npcIterator)->actionPoints += ap;

            if ((*npcIterator)->actionPoints > NP_MAX_AP) {
                (*npcIterator)->actionPoints = NP_MAX_AP;
            }

            (*npcIterator)->effects.checkEffects();

            if ((*npcIterator)->actionPoints > NP_MIN_AP && (*npcIterator)->getScript()) {
                // we have a script...
                // let's execute the command for this cycle
                std::shared_ptr<LuaNPCScript> npcscript = (*npcIterator)->getScript();
                npcscript->nextCycle();

                if ((*npcIterator)->getOnRoute() && !(*npcIterator)->waypoints.makeMove()) {
                    (*npcIterator)->setOnRoute(false);
                    npcscript->abortRoute();
                }
            }

            ++npcIterator;
        } // alive
        else {
            // Behandlung von toten NPC -> wiederbeleben
            (*npcIterator)->increaseAttrib("hitpoints", MAXHPS);
            sendSpinToAllVisiblePlayers((*npcIterator));
            ++npcIterator;
        }
    }
}


void World::workout_CommandBuffer(Player *&cp) {


}


bool World::ReadField(const char *inp, signed short int &outp) {
    char **error = NULL;
    long int temp=strtol(inp,error,10);

    if (error != NULL) {
        std::cerr << "ERROR in ReadField to signed short int" << std::endl;
    } else {
        if ((temp <= (0x7FFF)) && (temp >= -0x8000)) {
            outp = temp;
            return true;
        } else {
            std::cerr << "RANGE ERROR on ReadField to signed short int" << std::endl;
        }
    }

    return false;
}

bool World::ReadField(const char *inp, signed long int &outp) {
    char **error=NULL;
    signed long int temp=strtol(inp,error,10);

    if (error != NULL) {
        std::cerr << "ERROR in ReadField to signed long int" << std::endl;;
        return false;
    } else {
        outp = temp;
        return true;
    }
}



// Init method for NPC's
void World::initNPC() {
    Field *tempf;

    for (const auto &npc : Npc) {
        if (GetPToCFieldAt(tempf, npc->pos.x, npc->pos.y, npc->pos.z)) {
            tempf->removeChar();
        }

        sendRemoveCharToVisiblePlayers(npc->getId(), npc->pos);
        delete npc;
    }

    Npc.clear();
    NPCTable NPCTbl;
}
void World::initScheduler() {
    std::cout<<"Scheduler init \n";
    scheduler = new Scheduler(this);
    //===========Globale Tasks wie Wetter Gezeiteneffekte etc einfgen=========
    SchedulerObject *globalPlLearning;  //Task anlegen der die Geistige Aufnahmef�igkeit aller 10 sec bei Spielern wieder senkt
    globalPlLearning = new SGlobalPlayerLearnrate(scheduler->GetCurrentCycle()+5);
    scheduler->AddTask(globalPlLearning);
    SchedulerObject *globalMonLearning;  //Task anlegen der die Geistige Aufnahmef�igkeit aller 30 sec bei Monstern wieder senkt
    globalMonLearning = new SGlobalMonsterLearnrate(scheduler->GetCurrentCycle()+10);
    scheduler->AddTask(globalMonLearning);
    //=========================================================================
    std::cout<<"Scheduler init end \n";
}

