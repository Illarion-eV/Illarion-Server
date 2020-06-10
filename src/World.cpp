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

#include "World.hpp"

#include "Config.hpp"
#include "Logger.hpp"
#include "LongTimeAction.hpp"
#include "PlayerManager.hpp"
#include "Random.hpp"
#include "SchedulerTaskClasses.hpp"
#include "TableStructs.hpp"
#include "WaypointList.hpp"
#include "data/Data.hpp"
#include "data/MonsterTable.hpp"
#include "data/NPCTable.hpp"
#include "data/ScheduledScriptsTable.hpp"
#include "data/SkillTable.hpp"
#include "data/TilesTable.hpp"
#include "data/WeaponObjectTable.hpp"
#include "db/Result.hpp"
#include "db/SelectQuery.hpp"
#include "netinterface/BasicCommand.hpp"
#include "netinterface/NetInterface.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "script/LuaLogoutScript.hpp"
#include "script/LuaNPCScript.hpp"
#include "script/LuaWeaponScript.hpp"
#include "tuningConstants.hpp"

#include <algorithm>
#include <ctime>
#include <iterator>
#include <memory>
#include <regex>
#include <sys/types.h>

extern ScheduledScriptsTable *scheduledScripts;
extern MonsterTable *monsterDescriptions;
extern std::shared_ptr<LuaLogoutScript> logoutScript;
extern std::shared_ptr<LuaWeaponScript> standardFightingScript;

World *World::_self;

auto World::create() -> World * {
    if ((_self) == nullptr) {
        _self = new World();
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

auto World::get() -> World * {
    if ((_self) == nullptr) {
        throw std::runtime_error("world was not created");
    }

    return _self;
}

World::World() {
    lastTurnIGDay = getTime("day");

    usedAP = 0;

    // save starting time
    time_t starttime = 0;
    time(&starttime);
    timeStart = starttime * 1000;

    currentScript = nullptr;

    scriptDir = Config::instance().datadir() + std::string(SCRIPTSDIR);

    srand((unsigned)time(nullptr));
}

void World::turntheworld() {
    ftime(&now);
    unsigned long timeNow = now.time * 1000 + now.millitm;

    ap = timeNow / MIN_AP_UPDATE - timeStart / MIN_AP_UPDATE - usedAP;

    if (ap > 0) {
        usedAP += ap;

        checkPlayers();
        checkMonsters();
        checkNPC();
    }
}

void World::checkPlayers() {
    time_t tempkeepalive = 0;
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
                player.checkFightMode();
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

            try {
                fieldAt(pos).removePlayer();
            } catch (FieldNotFound &) {
            }

            Logger::info(LogFacility::Player) << "logout of " << player << Log::end;

            logoutScript->onLogout(playerPointer);

            PlayerManager::get().getLogOutPlayers().push_back(playerPointer);
            sendRemoveCharToVisiblePlayers(player.getId(), pos);
            lostPlayers.push_back(playerPointer);
        }
    });

    for (const auto &player : lostPlayers) {
        Players.erase(player->getId());
    }
}

void World::checkPlayerImmediateCommands() {
    std::unique_lock<std::mutex> lock(immediatePlayerCommandsMutex);
    while (!immediatePlayerCommands.empty()) {
        auto *player = immediatePlayerCommands.front();
        immediatePlayerCommands.pop();
        lock.unlock();

        if (player->Connection->online) {
            player->workoutCommands();
        }

        lock.lock();
    }
}

void World::addPlayerImmediateActionQueue(Player *player) {
    std::unique_lock<std::mutex> lock(immediatePlayerCommandsMutex);
    immediatePlayerCommands.push(player);
}

void World::invalidatePlayerDialogs() const { Players.for_each(&Player::invalidateDialogs); }

// init the respawn locations... for now still hardcoded...
auto World::initRespawns() -> bool {
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
                const auto spawnId = row["spp_id"].as<uint32_t>();
                const position pos(row["spp_x"].as<int16_t>(), row["spp_y"].as<int16_t>(), row["spp_z"].as<int16_t>());
                SpawnPoint newSpawn(pos, row["spp_range"].as<int>(), row["spp_spawnrange"].as<uint16_t>(),
                                    row["spp_minspawntime"].as<uint16_t>(), row["spp_maxspawntime"].as<uint16_t>(),
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

            bool foundMonster = monsterDescriptions->exists(monster.getMonsterType());
            const auto &monStruct = (*monsterDescriptions)[monster.getMonsterType()];

            if (monster.canAct()) {
                if (!monster.getOnRoute()) {
                    if (monster.getPosition() == monster.lastTargetPosition) {
                        monster.lastTargetSeen = false;
                    }

                    Item itl = monster.GetItemAt(LEFT_TOOL);
                    Item itr = monster.GetItemAt(RIGHT_TOOL);

                    uint16_t range = 1;

                    if (Data::WeaponItems.exists(itr.getId())) {
                        range = Data::WeaponItems[itr.getId()].Range;
                    } else if (Data::WeaponItems.exists(itl.getId())) {
                        range = Data::WeaponItems[itl.getId()].Range;
                    }

                    const auto temp = getTargetsInRange(monster.getPosition(), range);
                    bool has_attacked = false;
                    Character *target = nullptr;

                    if ((!temp.empty()) && monster.canAttack()) {
                        if (!monStruct.script || !monStruct.script->setTarget(monsterPointer, temp, target)) {
                            target = standardFightingScript->setTarget(monsterPointer, temp);
                        }

                        if (target != nullptr) {
                            monster.enemyid = target->getId();
                            monster.enemytype = Character::character_type(target->getType());
                            monster.lastTargetPosition = target->getPosition();
                            monster.lastTargetSeen = true;

                            if (foundMonster) {
                                if (monStruct.script) {
                                    if (monStruct.script->enemyNear(monsterPointer, target)) {
                                        return;
                                    }
                                }
                            } else {
                                Logger::error(LogFacility::Script)
                                        << "cant find a monster id for checking the script!" << Log::end;
                            }

                            monster.turn(target->getPosition());

                            if (monster.canFight()) {
                                has_attacked = characterAttacks(monsterPointer);
                            } else {
                                has_attacked = true;
                            }
                        }
                    }

                    if (!has_attacked) {
                        const auto targets = getTargetsInRange(monster.getPosition(), MONSTERVIEWRANGE);

                        bool makeRandomStep = true;

                        if ((!targets.empty()) && (monster.canAttack())) {
                            Character *targetChar = nullptr;

                            if (!monStruct.script ||
                                !monStruct.script->setTarget(monsterPointer, targets, targetChar)) {
                                targetChar = standardFightingScript->setTarget(monsterPointer, targets);
                            }

                            if (targetChar != nullptr) {
                                monster.lastTargetSeen = true;
                                monster.lastTargetPosition = targetChar->getPosition();

                                if (foundMonster) {
                                    if (monStruct.script) {
                                        if (monStruct.script->enemyOnSight(monsterPointer, targetChar)) {
                                            return;
                                        }
                                    }

                                    makeRandomStep = false;
                                    monster.performStep(targetChar->getPosition());
                                } else {
                                    Logger::notice(LogFacility::Script)
                                            << "cant find the monster id for calling a script!" << Log::end;
                                }
                            }
                        } else if (monster.lastTargetSeen) {
                            makeRandomStep = false;
                            monster.performStep(monster.lastTargetPosition);
                        }

                        if (makeRandomStep) {
                            int tempr = Random::uniform(1, 25);

                            bool hasDefinition = monsterDescriptions->exists(monster.getMonsterType());
                            const auto &monsterdef = (*monsterDescriptions)[monster.getMonsterType()];

                            if (!hasDefinition) {
                                Logger::error(LogFacility::World)
                                        << "Data for Healing not Found for monsterrace: " << monster.getMonsterType()
                                        << Log::end;
                            }

                            if (tempr <= 5 && hasDefinition && monsterdef.canselfheal) {
                                monster.heal();
                            } else {
                                SpawnPoint *spawn = monster.getSpawn();

                                auto dir = (direction)Random::uniform(0, 7);

                                if (spawn != nullptr) {
                                    position newpos = monster.getPosition();
                                    newpos.move(dir);
                                    int yoffs = spawn->get_y() - newpos.y;
                                    int xoffs = spawn->get_x() - newpos.x;

                                    // if walking out of range, mirroring dir. at spawn area border lets the char stay
                                    // in range with L_inf metric
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
                    }
                } else {
                    Item itl = monster.GetItemAt(LEFT_TOOL);
                    Item itr = monster.GetItemAt(RIGHT_TOOL);

                    uint16_t range = 1;

                    if (Data::WeaponItems.exists(itr.getId())) {
                        range = Data::WeaponItems[itr.getId()].Range;
                    } else if (Data::WeaponItems.exists(itl.getId())) {
                        range = Data::WeaponItems[itl.getId()].Range;
                    }

                    const auto temp = getTargetsInRange(monster.getPosition(), range);

                    if (!temp.empty()) {
                        Character *target = nullptr;

                        if (!monStruct.script || !monStruct.script->setTarget(monsterPointer, temp, target)) {
                            target = standardFightingScript->setTarget(monsterPointer, temp);
                        }

                        if (target != nullptr) {
                            if (foundMonster && monStruct.script) {
                                monStruct.script->enemyNear(monsterPointer, target);
                            } else {
                                Logger::error(LogFacility::World)
                                        << "cant find a monster id for checking the script!" << Log::end;
                            }
                        }
                    }

                    const auto temp2 = getTargetsInRange(monster.getPosition(), MONSTERVIEWRANGE);

                    if (!temp2.empty()) {
                        Character *target = nullptr;

                        if (!monStruct.script || !monStruct.script->setTarget(monsterPointer, temp2, target)) {
                            target = standardFightingScript->setTarget(monsterPointer, temp2);
                        }

                        if (target != nullptr) {
                            if (foundMonster && monStruct.script) {
                                monStruct.script->enemyOnSight(monsterPointer, target);
                            }
                        }
                    }

                    if (!monster.waypoints.makeMove()) {
                        monster.setOnRoute(false);

                        if (foundMonster && monStruct.script) {
                            monStruct.script->abortRoute(monsterPointer);
                        } else {
                            Logger::notice(LogFacility::Script)
                                    << "cant find the monster id for calling a script!" << Log::end;
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

        sendCharacterMoveToAllVisiblePlayers(monster, NORMALMOVE, 4);

        const auto monsterType = monster->getMonsterType();
        bool foundMonster = monsterDescriptions->exists(monsterType);
        const auto &monStruct = (*monsterDescriptions)[monsterType];

        if (foundMonster && monStruct.script) {
            monStruct.script->onSpawn(monster);
        }
    }

    newMonsters.clear();
}

auto World::getTargetsInRange(const position &pos, int radius) const -> std::vector<Character *> {
    Range range;
    range.radius = radius;
    range.zRadius = 0;
    const auto players = Players.findAllAliveCharactersInRangeOf(pos, range);
    const auto monsters = Monsters.findAllAliveCharactersInRangeOf(pos, range);
    std::vector<Character *> targets;
    targets.insert(targets.end(), players.begin(), players.end());
    std::remove_copy_if(monsters.begin(), monsters.end(), std::back_inserter(targets),
                        [&](const auto &monster) { return pos == monster->getPosition(); });

    return targets;
}

void World::checkNPC() {
    deleteAllLostNPC();

    Npc.for_each([this](NPC *npc) {
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

void World::workout_CommandBuffer(Player *&cp) {}

// Init method for NPC's
void World::initNPC() {
    Npc.for_each([this](NPC *npc) {
        try {
            fieldAt(npc->getPosition()).removeChar();
        } catch (FieldNotFound &) {
        }

        sendRemoveCharToVisiblePlayers(npc->getId(), npc->getPosition());
        delete npc;
    });

    Npc.clear();
    NPCTable NPCTbl;
}

// calculate when the next day change for illarion time will be
static auto getNextIGDayTime() -> std::chrono::steady_clock::time_point {
    // next day is at ((current unix timestamp - illarionBirthTime + (is_dst?3600:0)) / 28800 + 1) * 28800
    time_t curr_unixtime = time(nullptr);
    struct tm *timestamp = localtime(&curr_unixtime);
    static constexpr auto secondsInHour = 60 * 60;

    if (timestamp->tm_isdst != 0) {
        curr_unixtime += secondsInHour;
    }

    static constexpr auto realSecondsInIllarionDay = secondsInHour * 24 / illarionTimeFactor;
    curr_unixtime -= illarionBirthTime;
    curr_unixtime -= curr_unixtime % realSecondsInIllarionDay;
    curr_unixtime += realSecondsInIllarionDay;

    auto scheduler_ref = std::chrono::steady_clock::now();
    auto realtime_ref = std::chrono::system_clock::now();
    auto diff = std::chrono::system_clock::from_time_t(curr_unixtime) - realtime_ref;
    scheduler_ref += diff;

    return scheduler_ref;
}

void World::initScheduler() {
    scheduler.addRecurringTask([&] { Players.for_each(reduceMC); }, std::chrono::seconds(10),
                               "increase_player_learn_points");
    scheduler.addRecurringTask(
            [&] {
                Monsters.for_each(reduceMC);
                Npc.for_each(reduceMC);
            },
            std::chrono::seconds(10), "increase_monster_learn_points");
    scheduler.addRecurringTask([&] { monitoringClientList->CheckClients(); }, std::chrono::milliseconds(250),
                               "check_monitoring_clients");
    scheduler.addRecurringTask([&] { scheduledScripts->nextCycle(); }, std::chrono::seconds(1),
                               "check_scheduled_scripts");
    scheduler.addRecurringTask([&] { ageInventory(); }, std::chrono::minutes(3), "age_inventory");
    scheduler.addRecurringTask([&] { ageMaps(); }, std::chrono::minutes(3), "age_maps");
    scheduler.addRecurringTask([&] { turntheworld(); }, std::chrono::milliseconds(100), "turntheworld");
    scheduler.addRecurringTask([&] { sendIGTimeToAllPlayers(); }, std::chrono::hours(8), getNextIGDayTime(),
                               "update_ig_day");
}

auto World::executeUserCommand(Player *user, const std::string &input, const CommandMap &commands) -> bool {
    bool found = false;

    static const std::regex pattern("^!([^ ]+) ?(.*)?$");
    std::smatch match;

    if (std::regex_match(input, match, pattern)) {
        auto it = commands.find(match[1].str());

        if (it != commands.end()) {
            (it->second)(this, user, match[2].str());
            found = true;
        }
    }

    return found;
}
