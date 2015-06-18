//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.


#include "World.hpp"

#include <sstream>
#include <list>
#include <iostream>
#include <regex>

#include "Config.hpp"
#include "PlayerManager.hpp"
#include "Logger.hpp"
#include "constants.hpp"
#include "Player.hpp"
#include "Map.hpp"
#include "Monster.hpp"
#include "Field.hpp"

#include "data/Data.hpp"
#include "data/MonsterTable.hpp"
#include "data/RaceTypeTable.hpp"
#include "data/ScheduledScriptsTable.hpp"
#include "data/QuestNodeTable.hpp"

#include "script/LuaLookAtPlayerScript.hpp"
#include "script/LuaLookAtItemScript.hpp"
#include "script/LuaPlayerDeathScript.hpp"
#include "script/LuaWeaponScript.hpp"
#include "script/LuaReloadScript.hpp"
#include "script/LuaLearnScript.hpp"
#include "script/LuaLoginScript.hpp"
#include "script/LuaLogoutScript.hpp"
#include "script/LuaDepotScript.hpp"

#include "netinterface/protocol/ServerCommands.hpp"
#include "netinterface/NetInterface.hpp"

extern std::shared_ptr<LuaLookAtPlayerScript>lookAtPlayerScript;
extern std::shared_ptr<LuaLookAtItemScript>lookAtItemScript;
extern std::shared_ptr<LuaPlayerDeathScript>playerDeathScript;
extern std::shared_ptr<LuaLoginScript>loginScript;
extern std::shared_ptr<LuaLogoutScript>logoutScript;
extern std::shared_ptr<LuaLearnScript>learnScript;
extern std::shared_ptr<LuaDepotScript>depotScript;
extern std::unique_ptr<RaceTypeTable> raceTypes;
extern std::unique_ptr<MonsterTable> monsterDescriptions;
extern std::unique_ptr<ScheduledScriptsTable> scheduledScripts;
extern std::shared_ptr<LuaWeaponScript> standardFightingScript;

void set_spawn_command(World *, Player *, const std::string &);
void create_area_command(World *, Player *, const std::string &);
void set_login(World *, Player *, const std::string &);

// register any gm commands here...
void World::InitGMCommands() {

    GMCommands["what"] = [](World *world, Player *player, const std::string &) -> bool { world->what_command(player); return true; };

    GMCommands["?"] = [](World *world, Player *player, const std::string &) -> bool { world->gmhelp_command(player); return true; };

    GMCommands["warp_to"] = [](World *world, Player *player, const std::string &text) -> bool { world->warpto_command(player, text); return true; };
    GMCommands["w"] = GMCommands["warp_to"];

    GMCommands["summon"] = [](World *world, Player *player, const std::string &text) -> bool { world->summon_command(player, text); return true; };
    GMCommands["s"] = GMCommands["summon"];

    GMCommands["ban"] = [](World *world, Player *player, const std::string &text) -> bool { world->ban_command(player, text); return true; };
    GMCommands["b"] = GMCommands["ban"];

    GMCommands["tile"] = [](World *world, Player *player, const std::string &text) -> bool { world->tile_command(player, text); return true; };
    GMCommands["t"] = GMCommands["tile"];

    GMCommands["who"] = [](World *world, Player *player, const std::string &text) -> bool { world->who_command(player, text); return true; };

    GMCommands["turtleon"] = [](World *world, Player *player, const std::string &text) -> bool { world->turtleon_command(player, text); return true; };
    GMCommands["ton"] = GMCommands["turtleon"];
    GMCommands["turtleoff"] = [](World *world, Player *player, const std::string &) -> bool { world->turtleoff_command(player); return true; };
    GMCommands["toff"] = GMCommands["turtleoff"];

    GMCommands["clippingon"] = [](World *world, Player *player, const std::string &) -> bool { world->clippingon_command(player); return true; };
    GMCommands["con"] = GMCommands["clippingon"];
    GMCommands["clippingoff"] = [](World *world, Player *player, const std::string &) -> bool { world->clippingoff_command(player); return true; };
    GMCommands["coff"] = GMCommands["clippingoff"];

    GMCommands["playersave"] = [](World *world, Player *player, const std::string &) -> bool { world->playersave_command(player); return true; };
    GMCommands["ps"] = GMCommands["playersave"];

    GMCommands["add_teleport"] = [](World *world, Player *player, const std::string &text) -> bool { world->teleport_command(player, text); return true; };

    GMCommands["set_spawn"] = [](World *world, Player *player, const std::string &text) -> bool { set_spawn_command(world, player, text); return true; };

    GMCommands["create_area"] = [](World *world, Player *player, const std::string &text) -> bool { create_area_command(world, player, text); return true; };

    GMCommands["login"] = [](World *world, Player *player, const std::string &text) -> bool { world->set_login(player, text); return true; };

    GMCommands["forceintroduce"] = [](World *world, Player *player, const std::string &text) -> bool { world->ForceIntroduce(player, text); return true; };
    GMCommands["fi"] = GMCommands["forceintroduce"];
    GMCommands["forceintroduceall"] = [](World *world, Player *player, const std::string &) -> bool { world->ForceIntroduceAll(player); return true; };
    GMCommands["fia"] = GMCommands["forceintroduceall"];

    GMCommands["exportmaps"] = [](World *world, Player *player, const std::string &) -> bool { return world->exportMaps(player); };

    GMCommands["makeinvisible"] = [](World *world, Player *player, const std::string &) -> bool { world->makeInvisible(player); return true; };
    GMCommands["mi"] = GMCommands["makeinvisible"];

    GMCommands["makevisible"] = [](World *world, Player *player, const std::string &) -> bool { world->makeVisible(player); return true; };
    GMCommands["mv"] = GMCommands["makevisible"];

    GMCommands["showwarpfields"] = [](World *world, Player *player, const std::string &text) -> bool { world->showWarpFieldsInRange(player, text); return true; };

    GMCommands["removewarpfield"] = [](World *world, Player *player, const std::string &text) -> bool { world->removeTeleporter(player, text); return true; };

    GMCommands["talkto"] = [](World *world, Player *player, const std::string &text) -> bool { world->talkto_command(player, text); return true; };
    GMCommands["tt"] = GMCommands["talkto"];

    GMCommands["nuke"] = [](World *world, Player *player, const std::string &) -> bool { world->kill_command(player); return true; };

    GMCommands["fullreload"] = [](World *world, Player *player, const std::string &) -> bool { world->reload_command(player); return true; };
    GMCommands["fr"] = GMCommands["fullreload"];

    GMCommands["mapsave"] = [](World *world, Player *player, const std::string &) -> bool { world->save_command(player); return true; };

    GMCommands["jumpto"] = [](World *world, Player *player, const std::string &text) -> bool { world->jumpto_command(player, text); return true; };
    GMCommands["j"] = GMCommands["jumpto"];

    GMCommands["broadcast"] = [](World *world, Player *player, const std::string &text) -> bool { world->broadcast_command(player, text); return true; };
    GMCommands["bc"] = GMCommands["broadcast"];

    GMCommands["kickall"] = [](World *world, Player *player, const std::string &) -> bool { world->kickall_command(player); return true; };
    GMCommands["ka"] = GMCommands["kickall"];

    GMCommands["kick"] = [](World *world, Player *player, const std::string &text) -> bool { world->kickplayer_command(player, text); return true; };
    GMCommands["k"] = GMCommands["kick"];

    GMCommands["showips"] = [](World *world, Player *player, const std::string &) -> bool { world->showIPS_Command(player); return true; };
    GMCommands["create"] = [](World *world, Player *player, const std::string &text) -> bool { world->create_command(player, text); return true; };

    GMCommands["spawn"] = [](World *world, Player *player, const std::string &text) -> bool { world->spawn_command(player, text); return true; };

}

void World::spawn_command(Player *cp, const std::string &monid) {
    if (cp->hasGMRight(gmr_basiccommands)) {
        uint16_t id;
        std::stringstream ss;
        ss.str(monid);
        ss >> id;
        position pos = cp->getPosition();
        pos.x++;
        Logger::info(LogFacility::Admin) << *cp << " creates monster " << monid
                                         << " at " << pos << Log::end;
        createMonster(id, pos, 0);
    }
}

void World::create_command(Player *cp, const std::string &itemid) {
    if (cp->hasGMRight(gmr_basiccommands) || Config::instance().debug) {
        TYPE_OF_ITEM_ID item;
        uint16_t quantity = 1;
        uint16_t quality = 333;
        std::string data;
        std::string datalog;
        script_data_exchangemap dataList;

        std::stringstream ss;
        ss.str(itemid);
        ss >> item;
        ss >> quantity;
        ss >> quality;

        while (ss.good()) {
            ss >> data;
            size_t found = data.find('=');

            if (found != string::npos) {
                std::string key = data.substr(0, int(found));
                std::string value = data.substr(int(found) + 1);
                datalog += key + "=" + value + "; ";
                dataList.push_back(std::make_pair(key, value));
            }
        }

        Logger::info(LogFacility::Admin) << *cp << " creates item " << item << " with quantity "
                                         << quantity << ", quality " << quality << ", data " << datalog << Log::end;
        cp->createItem(item, quantity, quality, &dataList);
    }

}

void World::kill_command(Player *cp) {
    if (!cp->hasGMRight(gmr_reload)) {
        return;
    }

    uint32_t counter = 0;

    Monsters.for_each([&counter](Monster *monster) {
        monster->remove();
        ++counter;
    });

    std::string message = cp->to_string() + " nukes " + std::to_string(counter) + " monsters";
    Logger::info(LogFacility::Admin) << message << Log::end;
    sendMonitoringMessage(message);
}

void World::reload_command(Player *cp) {
    if (cp->hasGMRight(gmr_reload)) {
        std::string message = cp->to_string() + " issues a full reload";
        Logger::info(LogFacility::Admin) << message << Log::end;
        sendMonitoringMessage(message);

        if (reload_tables(cp)) {
            cp->inform("DB tables loaded successfully!");
        } else {
            cp->inform("CRITICAL ERROR: Failure while loading DB tables!");
        }
    }
}

void World::broadcast_command(Player *cp, const std::string &message) {
    if (cp->hasGMRight(gmr_broadcast)) {
        std::string logMsg = cp->to_string() + " broadcasts: " + message;
#ifdef LOG_TALK
        Logger::info(LogFacility::Player) << logMsg << Log::end;
#endif
        sendMonitoringMessage(logMsg);
        sendMessageToAllPlayers(message);
    }
}

void World::kickall_command(Player *cp) {
    if (cp->hasGMRight(gmr_forcelogout)) {
        std::string message = cp->to_string() + " kicks all players";
        Logger::info(LogFacility::Admin) << message << Log::end;
        sendMonitoringMessage(message);
        forceLogoutOfAllPlayers();
    }
}

void World::kickplayer_command(Player *cp, const std::string &player) {
    if (cp->hasGMRight(gmr_forcelogout)) {
        Logger::info(LogFacility::Admin) << *cp << " kicks " << player << Log::end;
        forceLogoutOfPlayer(player);
    }
}

void World::showIPS_Command(Player *cp) {
    if (cp->hasGMRight(gmr_basiccommands)) {
        Logger::info(LogFacility::Admin) << *cp << " requests player info" << Log::end;
        sendAdminAllPlayerData(cp);
    }
}

void World::jumpto_command(Player *cp,const std::string &player) {
    if (cp->hasGMRight(gmr_warp) || Config::instance().debug) {
        cp->closeAllShowcasesOfMapContainers();
        teleportPlayerToOther(cp, player);
        Logger::info(LogFacility::Admin) << *cp << " jumps to player " << player
                                         << " at " << cp->getPosition() << Log::end;
    }
}

void World::save_command(Player *cp) {
    if (!cp->hasGMRight(gmr_save)) {
        return;
    }

    Logger::info(LogFacility::Admin) << *cp << " saves all maps" << Log::end;

    Players.for_each([this](Player *player) {
        try {
            fieldAt(player->getPosition()).removePlayer();
        } catch (FieldNotFound &) {
        }
    });

    Save();

    Players.for_each([this](Player *player) {
        try {
            fieldAt(player->getPosition()).setPlayer();
        } catch (FieldNotFound &) {
        }
    });

    std::string tmessage = "*** Maps saved! ***";
    cp->inform(tmessage);
}

void World::talkto_command(Player *player, const std::string &text) {
    if (!player->hasGMRight(gmr_basiccommands)) {
        return;
    }

    static const std::regex pattern("^(.*?),(.*)$");
    std::smatch match;

    if (std::regex_match(text, match, pattern)) {
        Player *target = Players.find(match[1].str());

        if (target) {
#ifdef LOG_TALK
            Logger::info(LogFacility::Player) << *player << " talks to " << *target << ": " << match[2].str() << Log::end;
#endif
            target->inform(match[2].str(), Player::informGM);
            std::string message = "to " + target->to_string() + ": " + match[2].str();
            player->inform(message, Player::informScriptMediumPriority);
        }
    }
}

void World::makeInvisible(Player *cp) {
    if (!cp->hasGMRight(gmr_visible)) {
        return;
    }

    cp->setInvisible(true);
    Logger::info(LogFacility::Admin) << *cp << " becomes invisible" << Log::end;
    sendRemoveCharToVisiblePlayers(cp->getId(), cp->getPosition());
}

void World::makeVisible(Player *cp) {
    if (!cp->hasGMRight(gmr_visible)) {
        return;
    }

    cp->setInvisible(false);

    Logger::info(LogFacility::Admin) << *cp << " becomes visible" << Log::end;

    for (const auto &player : Players.findAllCharactersInScreen(cp->getPosition())) {
        if (cp != player) {
            ServerCommandPointer cmd = std::make_shared<MoveAckTC>(cp->getId(), cp->getPosition(), PUSH, 0);
            player->Connection->addCommand(cmd);
        }
    }

    ServerCommandPointer cmd = std::make_shared<AppearanceTC>(cp, cp);
    cp->Connection->addCommand(cmd);
}

void World::ForceIntroduce(Player *player, const std::string &text) {
    if (!player->hasGMRight(gmr_basiccommands)) {
        return;
    }

    auto target = Players.find(text);

    if (target) {
        forceIntroducePlayer(target, player);
    }
}

void World::ForceIntroduceAll(Player *player) {
    if (!player->hasGMRight(gmr_basiccommands)) {
        return;
    }

    Range range;
    range.radius = player->getScreenRange();

    for (const auto &p : Players.findAllCharactersInRangeOf(player->getPosition(), range)) {
        if (player->getId() != p->getId()) {
            forceIntroducePlayer(p, player);
        }
    }
}

void World::teleportPlayerToOther(Player *player, std::string text) {
    if (!player->hasGMRight(gmr_warp)) {
        return;
    }

    auto target = Players.find(text);

    if (target && target->getId() != player->getId()) {
        player->Warp(target->getPosition());
    }
}


void World::forceLogoutOfAllPlayers() {
    Players.for_each([this](Player *player) {
        try {
            fieldAt(player->getPosition()).removePlayer();
        } catch (FieldNotFound &) {
        }

        std::string message = "--- kicked: ";
        message = message + player->to_string();
        Logger::info(LogFacility::Admin) << message << Log::end;
        sendMonitoringMessage(message);
        ServerCommandPointer cmd = std::make_shared<LogOutTC>(SERVERSHUTDOWN);
        player->Connection->shutdownSend(cmd);
        PlayerManager::get().getLogOutPlayers().push_back(player);
    });

    Players.clear();
}


bool World::forceLogoutOfPlayer(const std::string &name) {
    Player *temp = Players.find(name);

    if (temp) {
        std::string message = "--- kicked: ";
        message = message + temp->to_string();
        Logger::info(LogFacility::Admin) << message << Log::end;
        sendMonitoringMessage(message);
        ServerCommandPointer cmd = std::make_shared<LogOutTC>(BYGAMEMASTER);
        temp->Connection->shutdownSend(cmd);
        return true;
    } else {
        return false;
    }
}


void World::sendAdminAllPlayerData(Player *admin) {
    if (!admin->hasGMRight(gmr_basiccommands)) {
        return;
    }

    ServerCommandPointer cmd = std::make_shared<AdminViewPlayersTC>();
    admin->Connection->addCommand(cmd);
}


void World::warpto_command(Player *p, const std::string &text) {
    if (!p->hasGMRight(gmr_warp) && !Config::instance().debug) {
        return;
    }

    static const std::regex pattern(R"(^(-?\d+)[ ,]?(-?\d+)?[ ,]?(-?\d+)?$)");
    std::smatch match;

    if (std::regex_match(text, match, pattern)) {
        try {
            position warpto = p->getPosition();
            auto a = boost::lexical_cast<short int>(match[1].str());

            if (!match[2].str().empty()) {
                auto b = boost::lexical_cast<short int>(match[2].str());

                if (!match[3].str().empty()) {
                    auto c = boost::lexical_cast<short int>(match[3].str());
                    warpto = {a, b, c};
                } else {
                    warpto.x = a;
                    warpto.y = b;
                }
            } else {
                warpto.z = a;
            }

            p->forceWarp(warpto);
            Logger::info(LogFacility::Admin) << *p << " warps to " << warpto << Log::end;
        } catch (boost::bad_lexical_cast &) {
        }
    }
}


// !summon <player>
void World::summon_command(Player *player, const std::string &text) {
    if (!player->hasGMRight(gmr_summon)) {
        return;
    }

    auto target = Players.find(text);

    if (target && target->getId() != player->getId()) {
        Logger::info(LogFacility::Admin) << *player << " summons player " << *target << " to " << player->getPosition() << Log::end;
        target->Warp(player->getPosition());
    }
}


// !ban <time> [m|h|d] <player>
void World::ban_command(Player *cp, const std::string &text) {
    if (!cp->hasGMRight(gmr_ban)) {
        return;
    }

    static const std::regex pattern(R"(^(?:(\d+) ?([mhd]) ?)?(.+)$)");
    std::smatch match;

    if (std::regex_match(text, match, pattern)) {
        auto target = Players.find(match[3].str());

        if (target) {
            if (match[1].str().empty()) {
                ban(target, 0, cp->getId());
                std::string message = cp->to_string() + " bans player " + target->to_string() + " indefinately";
                Logger::info(LogFacility::Admin) << message << Log::end;
                sendMonitoringMessage(message);
                message = "*** Banned player " + target->to_string() + " indefinately";
                cp->inform(message);
            } else {
                try {
                    auto duration = boost::lexical_cast<int>(match[1].str());
                    char timeunit = match[2].str()[0];

                    switch (timeunit) {
                    case 'd':
                        ban(target, duration*86400, cp->getId());
                        break;
                    case 'h':
                        ban(target, duration*3600, cp->getId());
                        break;
                    case 'm':
                        ban(target, duration*60, cp->getId());
                        break;
                    default:
                        break;
                    }

                    std::string message = cp->to_string() + " bans player " + target->to_string() + " for " + std::to_string(duration) + timeunit;
                    Logger::info(LogFacility::Admin) << message << Log::end;
                    sendMonitoringMessage(message);
                    message = "*** Banned player " + target->to_string() + " for " + match[1].str() + match[2].str();
                    cp->inform(message);
                } catch (boost::bad_lexical_cast &) {
                    cp->inform("*** Invalid duration, player not banned!");
                }
            }
        } else {
            std::string message = "*** Could not find " + match[3].str();
            cp->inform(message);
        }
    }
}


void World::ban(Player *cp, int bantime, TYPE_OF_CHARACTER_ID gmid) {
    if (bantime == 0) {
        cp->SetStatus(BANNED);
        cp->SetStatusTime(0);
        cp->SetStatusGM(gmid);
    } else if (bantime > 0) {
        cp->SetStatus(BANNEDFORTIME);
        time_t ttime;
        time(&ttime);
        // Banned for seconds
        cp->SetStatusTime(ttime + bantime);
        cp->SetStatusGM(gmid);
    }

    forceLogoutOfPlayer(cp->getName());
}


// !who [player]
void World::who_command(Player *cp, const std::string &tplayer) {
    if (!cp->hasGMRight(gmr_basiccommands) && !Config::instance().debug) {
        return;
    }

    if (tplayer == "") {

        std::string tmessage = "";

        Players.for_each([&tmessage](Player *p) {
            if (tmessage.length() > 0) {
                tmessage = tmessage + ", ";
            }

            tmessage = tmessage + p->to_string();
        });

        if (tmessage.length() > 0) {
            cp->inform(tmessage);
        }
    } else {

        Player *tempPl = Players.find(tplayer);

        if (tempPl) {
            std::string tmessage = tempPl->to_string();
            const auto &pos = tempPl->getPosition();

            tmessage = tmessage + " x" + std::to_string(pos.x);
            tmessage = tmessage + " y" + std::to_string(pos.y);
            tmessage = tmessage + " z" + std::to_string(pos.z);
            tmessage = tmessage + " HPs:" + std::to_string(tempPl->getAttribute(Character::hitpoints));
            tmessage = tmessage + ((tempPl->isAlive()) ? " Alive" : " Dead");
            std::string german = " German";
            std::string english = " English";
            tmessage = tmessage + tempPl->nls(german, english);

            cp->inform(tmessage);
        }
    }
}


void World::tile_command(Player *cp, const std::string &tile) {
    if (!cp->hasGMRight(gmr_settiles)) {
        return;
    }

    try {
        setNextTile(cp, boost::lexical_cast<unsigned short>(tile));
    } catch (boost::bad_lexical_cast &) {
    }
}


void World::setNextTile(Player *cp, unsigned char tilenumber) {
    const position &pos = cp->getFrontalPosition();

    try {
        Field &field = fieldAt(pos);
        field.setTileId(tilenumber);
    } catch (FieldNotFound &) {
    }

    //update the current area
    cp->sendRelativeArea(0);
    sendAllVisibleCharactersToPlayer(cp, true);

}


void World::turtleon_command(Player *cp, const std::string &tile) {
    if (!cp->hasGMRight(gmr_settiles)) {
        return;
    }

    try {
        auto id = boost::lexical_cast<unsigned short>(tile);
        cp->setTurtleActive(true);
        cp->setTurtleTile(id);
    } catch (boost::bad_lexical_cast &) {
    }
}


void World::turtleoff_command(Player *cp) {
    if (!cp->hasGMRight(gmr_settiles)) {
        return;
    }

    cp->setTurtleActive(false);
}


void World::clippingon_command(Player *cp) {
    if (!cp->hasGMRight(gmr_clipping)) {
        return;
    }

    Logger::info(LogFacility::Admin) << *cp << " turns on clipping" << Log::end;
    cp->setClippingActive(true);
}


void World::clippingoff_command(Player *cp) {
    if (!cp->hasGMRight(gmr_clipping)) {
        return;
    }

    Logger::info(LogFacility::Admin) << *cp << " turns off clipping" << Log::end;
    cp->setClippingActive(false);
}


void World::what_command(Player *cp) {
    position front = cp->getFrontalPosition();

    cp->inform("Facing:");
    std::stringstream message;

    message << "- Position " << front;
    cp->inform(message.str());

    try {
        Field &field = fieldAt(front);
        message.str("");

        message << "- Tile " << field.getTileId();
        cp->inform(message.str());
        Item top;

        if (field.viewItemOnStack(top)) {
            message.str("");

            message << "- Item " << top.getId();

            if (cp->hasGMRight(gmr_basiccommands) || Config::instance().debug) {
                message << ", Stack of " << top.getNumber();
                message << ", Quality " << top.getQuality();

                if (top.getDataBegin() != top.getDataEnd()) {
                    message << ", Data";

                    for (auto it = top.getDataBegin(); it != top.getDataEnd(); ++it) {
                        message << " '" << it->first << "'='" << it->second << "'";
                    }
                }

                message << ", Wear " << (uint16_t)top.getWear();
            }

            cp->inform(message.str());
        }

        Character *character = findCharacterOnField(front);

        if (character) {
            message.str("");
            uint32_t id = character->getId();

            if (id >= DYNNPC_BASE) {
                message << "- Dynamic NPC";
            } else if (id >= NPC_BASE) {
                message << "- NPC " << id-NPC_BASE;
            } else if (id >= MONSTER_BASE) {
                message << "- Monster " << dynamic_cast<Monster *>(character)->getMonsterType();
            } else {
                message << "- Player";
            }

            cp->inform(message.str());
        }
    } catch (FieldNotFound &) {
    }
}


void World::playersave_command(Player *cp) {
    if (!cp->hasGMRight(gmr_save)) {
        return;
    }

    Logger::info(LogFacility::Admin) << *cp << " saves all players" << Log::end;

    Players.for_each(&Player::save);

    std::string tmessage = "*** All online players saved! ***";
    cp->inform(tmessage);

}


// !teleport X<,| >Y[<,| >Z]
void World::teleport_command(Player *cp, const std::string &text) {
    if (!cp->hasGMRight(gmr_warp)) {
        return;
    }

    static const std::regex pattern(R"(^(-?\d+)[, ](-?\d+)[, ](-?\d+)$)");
    std::smatch match;

    if (std::regex_match(text, match, pattern)) {
        try {
            auto x = boost::lexical_cast<short int>(match[1].str());
            auto y = boost::lexical_cast<short int>(match[2].str());
            auto z = boost::lexical_cast<short int>(match[3].str());

            if (addWarpField(cp->getPosition(), position(x, y, z), 0, 0)) {
                cp->inform("*** warp field added ***");
            } else {
                cp->inform("*** Error: could not add warp field ***");
            }
        } catch (boost::bad_lexical_cast &) {
            cp->inform("*** Error: could not parse target position ***");
        }
    }
}


void World::gmhelp_command(Player *cp) {
    if (!cp->hasGMRight(gmr_basiccommands)) {
        if (Config::instance().debug) {
            std::string tmessage = " <> - parameter.  [] - optional.  | = choice.  () = shortcut";
            cp->inform(tmessage);
            tmessage = "!create <id> [<quantity> [<quality> [[<data_key>=<data_value>] ...]]] creates an item in your inventory.";
            cp->inform(tmessage);
            tmessage = "!jumpto <player> - (!j) teleports you to the player.";
            cp->inform(tmessage);
            tmessage = "!warp <x> <y> [<z>] | !warp <z> - (!w) change given coordinates.";
            cp->inform(tmessage);
            tmessage = "!what - sends various information of the field or the character in front of you.";
            cp->inform(tmessage);
            tmessage = "!who [<player>] - List all players online or a single player if specified.";
            cp->inform(tmessage);
        } else {
            std::string tmessage = "!what - sends various information of the field or the character in front of you.";
            cp->inform(tmessage);
        }

        return;
    }

    std::string tmessage = " <> - parameter.  [] - optional.  | = choice.  () = shortcut";
    cp->inform(tmessage);

    if (cp->hasGMRight(gmr_basiccommands)) {
        tmessage = "!what - sends various information of the field or the character in front of you.";
        cp->inform(tmessage);
        tmessage = "!who [<player>] - List all players online or a single player if specified.";
        cp->inform(tmessage);
        tmessage = "!forceintroduce <char id|char name> - (!fi) introduces the char to all gms in range.";
        cp->inform(tmessage);
        tmessage = "!forceintroduceall - (!fia) introduces all chars in sight to you.";
        cp->inform(tmessage);
        tmessage = "!talkto <player>, <message> - (!tt) sends a message to a specific player important is the , after the id or name!";
        cp->inform(tmessage);
        tmessage = "!broadcast <message> - (!bc) Broadcasts the message <message> to all players IG.";
        cp->inform(tmessage);
        tmessage = "!create <id> [<quantity> [<quality> [[<data_key>=<data_value>] ...]]] creates an item in your inventory.";

    }

    if (cp->hasGMRight(gmr_warp)) {
        tmessage = "!warp <x> <y> [<z>] | !warp <z> - (!w) change given coordinates.";
        cp->inform(tmessage);
        tmessage = "!add_teleport <x> <y> <z> - Adds a teleportfield from your position to the field <x> <y> <z>.";
        cp->inform(tmessage);
        tmessage = "!showwarpfields <range> - Shows all warpfields in the range <range>.";
        cp->inform(tmessage);
        tmessage = "!removewarpfield <x> <y> <z> - Removes the warpfield at the position <x> <y> <z>.";
        cp->inform(tmessage);
        tmessage = "!jumpto <player> - (!j) teleports you to the player.";
        cp->inform(tmessage);

    }

    if (cp->hasGMRight(gmr_summon)) {
        tmessage = "!summon <player> - (!s) Summons a player to you.";
        cp->inform(tmessage);
    }

    if (cp->hasGMRight(gmr_ban)) {
        tmessage = "!ban <time> [<m|h|d>] <player> - (!b) Bans the player <player> for <time> [m]inutes/[h]ours/[d]ays.";
        cp->inform(tmessage);
    }

    if (cp->hasGMRight(gmr_settiles)) {
        tmessage = "!tile <tilenumber> - (!t) changes the tile in front of you to the tile with id <tilenumber>.";
        cp->inform(tmessage);
        tmessage = "!turtleon <tile number> - (!ton) Change tiles to tile # when you walk (advanced)";
        cp->inform(tmessage);
        tmessage = "!turtleoff - (!toff) Stop changing tiles as you walk.";
        cp->inform(tmessage);
    }

    if (cp->hasGMRight(gmr_clipping)) {
        tmessage = "!clippingon - (!con) Turn clipping on (Can't walk through walls)";
        cp->inform(tmessage);
        tmessage = "!clippingoff - (!coff) Turn clipping off (Walk anywhere)";
        cp->inform(tmessage);
    }

    if (cp->hasGMRight(gmr_save)) {
        tmessage = "!playersave - (!ps) saves all players to the database.";
        cp->inform(tmessage);
        tmessage = "!mapsave - Saves the map after changes where made.";
        cp->inform(tmessage);
    }

    if (cp->hasGMRight(gmr_reload)) {
        tmessage = "!set_spawn <true|false> - activates/deactivates the spawning of monsters.";
        cp->inform(tmessage);
        tmessage = "!reloaddefinitions - (!rd) reloads all datas without spawnpoints, so no new monsters are spawned. (deactivated)";
        cp->inform(tmessage);
        tmessage = "!nuke - kills all Monster on the map (to clean the map after a spawn reload).";
        cp->inform(tmessage);
        tmessage = "!fullreload - (!fr) reloads all database tables";
        cp->inform(tmessage);
    }

    if (cp->hasGMRight(gmr_import)) {
        tmessage = "create_area <x> <y> <z> <width> <height> <tile> - Creates a new map at <x> <y> <z> with the dimensions <height> <width> and filled with <tile>.";
        cp->inform(tmessage);
        tmessage = "!exportmaps - Exports the current maps.";
        cp->inform(tmessage);
    }

    if (cp->hasGMRight(gmr_loginstate)) {
        tmessage = "!login <true|false> - changes the login state, with false only gm's can log in.";
        cp->inform(tmessage);
    }

    if (cp->hasGMRight(gmr_visible)) {
        tmessage = "!makeinvisible - (!mi) makes you invisible for other chars. NOT FOR MONSTERS.";
        cp->inform(tmessage);
        tmessage = "!makevisible - (!mv) makes you visible if you are invisible.";
        cp->inform(tmessage);
    }

    if (cp->hasGMRight(gmr_forcelogout)) {
        tmessage = "!kickall - Kicks all players out of the game.";
        cp->inform(tmessage);
        tmessage = "!kick <player> - Kicks the player with the id out of the game.";
        cp->inform(tmessage);
    }
}

//! parse GMCommands of the form !<string1> <string2> and process them
bool World::parseGMCommands(Player *user, const std::string &text) {
    return executeUserCommand(user, text, GMCommands);
}

void reportError(Player *cp, std::string msg) {
    Logger::error(LogFacility::World) << "ERROR: " << msg << Log::end;
    cp->inform("ERROR: " + msg);
}

void reportScriptError(Player *cp, std::string serverscript, std::string what) {
    reportError(cp, "Failed to reload server." + serverscript + ": " + what);
}

void reportTableError(Player *cp, std::string dbtable) {
    reportError(cp, "Failed to reload DB table: " + dbtable);
}


bool World::reload_defs(Player *cp) {
    if (!cp->hasGMRight(gmr_reload)) {
        return false;
    }

    Data::preReload();

    sendMessageToAllPlayers("### The server is reloading, this may cause some lag ###");

    bool ok = Data::Skills.reloadBuffer();

    if (ok) {
        Data::Skills.activateBuffer();
    }

    std::unique_ptr<MonsterTable> monsterDescriptionsTemp;
    std::unique_ptr<RaceTypeTable> raceTypesTemp;
    std::unique_ptr<ScheduledScriptsTable> scheduledScriptsTemp;

    if (ok) {
        QuestNodeTable::getInstance().reload();
    }

    if (ok) {
        ok = Data::reload();
    }

    if (ok) {
        monsterDescriptionsTemp = std::make_unique<MonsterTable>();

        if (!monsterDescriptionsTemp || !monsterDescriptionsTemp->isDataOK()) {
            reportTableError(cp, "monster");
            ok = false;
        }
    }

    if (ok) {
        raceTypesTemp = std::make_unique<RaceTypeTable>();

        if (!raceTypesTemp || !raceTypesTemp->isDataOK()) {
            reportTableError(cp, "race_types");
            ok = false;
        }
    }

    if (ok) {
        scheduledScriptsTemp = std::make_unique<ScheduledScriptsTable>();

        if (!scheduledScriptsTemp || !scheduledScriptsTemp->isDataOK()) {
            reportTableError(cp, "scheduledscripts");
            ok = false;
        }
    }

    if (ok) {
        // if everything went well, delete old tables and set up new tables
        //Mutex für login logout sperren so das aktuell keiner mehr einloggen kann
        PlayerManager::get().setLoginLogout(true);
        monsterDescriptions = std::move(monsterDescriptionsTemp);
        raceTypes = std::move(raceTypesTemp);
        scheduledScripts = std::move(scheduledScriptsTemp);
        //Mutex entsperren.
        PlayerManager::get().setLoginLogout(false);

        //Reload the standard Fighting script
        try {
            std::shared_ptr<LuaWeaponScript> tmpScript = std::make_shared<LuaWeaponScript>("server.standardfighting");
            standardFightingScript = tmpScript;
        } catch (ScriptException &e) {
            reportScriptError(cp, "standardfighting", e.what());
            ok = false;
        }

        try {
            std::shared_ptr<LuaLookAtPlayerScript>tmpScript = std::make_shared<LuaLookAtPlayerScript>("server.playerlookat");
            lookAtPlayerScript = tmpScript;
        } catch (ScriptException &e) {
            reportScriptError(cp, "playerlookat", e.what());
            ok = false;
        }

        try {
            std::shared_ptr<LuaLookAtItemScript>tmpScript = std::make_shared<LuaLookAtItemScript>("server.itemlookat");
            lookAtItemScript = tmpScript;
        } catch (ScriptException &e) {
            reportScriptError(cp, "itemlookat", e.what());
            ok = false;
        }

        try {
            std::shared_ptr<LuaPlayerDeathScript>tmpScript = std::make_shared<LuaPlayerDeathScript>("server.playerdeath");
            playerDeathScript = tmpScript;
        } catch (ScriptException &e) {
            reportScriptError(cp, "playerdeath", e.what());
            ok = false;
        }

        try {
            std::shared_ptr<LuaLoginScript>tmpScript = std::make_shared<LuaLoginScript>("server.login");
            loginScript = tmpScript;
        } catch (ScriptException &e) {
            reportScriptError(cp, "login", e.what());
            ok = false;
        }

        try {
            std::shared_ptr<LuaLogoutScript>tmpScript = std::make_shared<LuaLogoutScript>("server.logout");
            logoutScript = tmpScript;
        } catch (ScriptException &e) {
            reportScriptError(cp, "logout", e.what());
            ok = false;
        }

        try {
            std::shared_ptr<LuaLearnScript>tmpScript = std::make_shared<LuaLearnScript>("server.learn");
            learnScript = tmpScript;
        } catch (ScriptException &e) {
            reportScriptError(cp, "learn", e.what());
            ok = false;
        }

        try {
            std::shared_ptr<LuaDepotScript>tmpScript = std::make_shared<LuaDepotScript>("server.depot");
            depotScript = tmpScript;
        } catch (ScriptException &e) {
            reportScriptError(cp, "depot", e.what());
            ok = false;
        }

    }


    if (ok) {
        cp->inform(" *** Definitions reloaded *** ");
    } else {
        cp->inform("CRITICAL ERROR: Failure while reloading definitions");
    }

    return ok;
}


bool World::reload_tables(Player *cp) {

    LuaScript::shutdownLua();

    bool ok = reload_defs(cp);

    if (ok) {
        // reload respawns
        initRespawns();

        //reload NPC's
        initNPC();

        Players.for_each(&Player::sendCompleteQuestProgress);

        try {
            std::shared_ptr<LuaReloadScript> tmpScript = std::make_shared<LuaReloadScript>("server.reload");
            tmpScript->onReload();
        } catch (ScriptException &e) {
            reportScriptError(cp, "reload", e.what());
        }

    }

    return ok;
}


// enable/disable spawnpoints
void set_spawn_command(World *world, Player *player, const std::string &in) {
    if (!player->hasGMRight(gmr_reload)) {
        return;
    }

    bool enable = false;

    if (in == "true") {
        enable = true;
    }

    Logger::info(LogFacility::Admin) << *player << " sets spawn to " << enable << Log::end;

    world->enableSpawn(enable);
}


// create a new area starting at x,y,z with dimension w,h, filltile ft (create_area x y z w h ft)
void create_area_command(World *world, Player *player,const std::string &params) {
    if (!player->hasGMRight(gmr_import)) {
        return;
    }

    std::stringstream ss(params);
    int x,y,z,w,h, tile;
    x=y=z=w=h=tile=-65535;
    ss >> x;
    ss >> y;
    ss >> z;
    ss >> w;
    ss >> h;
    ss >> tile;

    if (x==-65535 || y == -65535 || z == -65535 || w < 1 || h < 1 || tile < 0) {
        Logger::error(LogFacility::World) << "Error in create_area_command issued by " << *player << "; input: " << x << " " << y << " " << z << " " << w << " " << h << Log::end;
        return;
    }

    if (world->maps.createMap("by " + player->to_string(), position(x, y, z), w,
                              h, tile)) {
        std::string tmessage = "Map inserted.";
        player->inform(tmessage);
        Logger::info(LogFacility::World) << "Map created by " << *player
                                         << " on " << x << " - " << y << " - "
                                         << z << " with w: " << w << " h: " << h
                                         << " tile: " << tile << Log::end;
    } else {
        std::string tmessage = "Failed to insert map.";
        player->inform(tmessage);
    }
}

void World::set_login(Player *player, const std::string &text) {
    if (!player->hasGMRight(gmr_loginstate)) {
        return;
    }

    bool enable = true;

    if (text == "false") {
        enable = false;
    }

    allowLogin(enable);
    std::string message = player->to_string() + " set login flag to " + (enable ? "true" : "false");
    Logger::info(LogFacility::Admin) << message << Log::end;
    sendMonitoringMessage(message);
    message = "login flag set to: ";
    message += enable ? "true" : "false";
    player->inform(message);
}

bool World::exportMaps(Player *cp) {
    if (!cp->hasGMRight(gmr_import)) {
        return false;
    }

    std::string exportDir = directory + std::string(MAPDIR) + "export/";
    return maps.exportTo(exportDir);
}

void World::removeTeleporter(Player *cp, const std::string &text) {
    if (!cp->hasGMRight(gmr_warpfields)) {
        return;
    }

    static const std::regex pattern(R"(^(-?\d+)[, ](-?\d+)[, ](-?\d+)$)");
    std::smatch match;

    if (std::regex_match(text, match, pattern)) {
        try {
            auto x = boost::lexical_cast<short int>(match[1].str());
            auto y = boost::lexical_cast<short int>(match[2].str());
            auto z = boost::lexical_cast<short int>(match[3].str());

            if (removeWarpField(position(x, y, z))) {
                cp->inform("*** warp field deleted ***");
            } else {
                cp->inform("*** Error: could not delete warp field ***");
            }
        } catch (boost::bad_lexical_cast &) {
            cp->inform("*** Error: could not parse warp field position ***");
        }
    }
}

void World::showWarpFieldsInRange(Player *cp, const std::string &text) {
    if (!cp->hasGMRight(gmr_warpfields)) {
        return;
    }

    try {
        auto range = boost::lexical_cast<short int>(text);
        std::vector<position> warpfieldsinrange;

        if (findWarpFieldsInRange(cp->getPosition(), range, warpfieldsinrange)) {
            cp->inform("List of warp fields:");

            for (const auto &warpfield : warpfieldsinrange) {
                try {
                    position target;
                    fieldAt(warpfield).getWarp(target);
                    std::string message = warpfield.toString() + " -> " + target.toString();
                    cp->inform(message);
                } catch (FieldNotFound &) {
                }
            }

            cp->inform("---");
        }
    } catch (boost::bad_lexical_cast &) {
        cp->inform("*** Error: could not parse range ***");
    }
}

