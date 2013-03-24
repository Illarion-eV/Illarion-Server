//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.


#include "World.hpp"
#include <sstream>
#include <regex.h>
#include <list>
#include "data/Data.hpp"
#include "script/LuaLookAtPlayerScript.hpp"
#include "script/LuaLookAtItemScript.hpp"
#include "script/LuaPlayerDeathScript.hpp"
#include "data/MonsterTable.hpp"
#include "data/ScheduledScriptsTable.hpp"
//We need this for the standard Fighting Script.
#include "script/LuaWeaponScript.hpp"
//For the reload scripts
#include "script/LuaReloadScript.hpp"
#include "script/LuaLearnScript.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "netinterface/NetInterface.hpp"
#include "script/LuaLoginScript.hpp"
#include "script/LuaLogoutScript.hpp"
#include "script/LuaDepotScript.hpp"
#include "PlayerManager.hpp"
#include "Logger.hpp"
#include "data/QuestNodeTable.hpp"
#include "constants.hpp"

#include <iostream>

extern std::shared_ptr<LuaLookAtPlayerScript>lookAtPlayerScript;
extern std::shared_ptr<LuaLookAtItemScript>lookAtItemScript;
extern std::shared_ptr<LuaPlayerDeathScript>playerDeathScript;
extern std::shared_ptr<LuaLoginScript>loginScript;
extern std::shared_ptr<LuaLogoutScript>logoutScript;
extern std::shared_ptr<LuaLearnScript>learnScript;
extern std::shared_ptr<LuaDepotScript>depotScript;

void set_spawn_command(World *, Player *, const std::string &);
void create_area_command(World *, Player *, const std::string &);
void set_login(World *, Player *, const std::string &);

template< typename To, typename From> To stream_convert(const From &from) {
    std::stringstream stream;
    stream << from;
    To to;
    stream >> to;
    return to;
}

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

    GMCommands["nologin"] = [](World *world, Player *player, const std::string &text) -> bool { set_login(world, player, text); return true; };

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
        position pos = cp->pos;
        pos.x++;
        Logger::info(LogFacility::Admin) << cp->to_string() << " creates monster " << monid
                                         << " at " << pos.toString() << Log::end;
        createMonster(id, pos, 0);
    }
}

void World::create_command(Player *cp, const std::string &itemid) {
#ifndef TESTSERVER

    if (cp->hasGMRight(gmr_basiccommands))
#endif
    {
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

        Logger::info(LogFacility::Admin) << cp->to_string() << " creates item " << item << " with quantity "
                                         << quantity << ", quality " << quality << ", data " << datalog << Log::end;
        cp->createItem(item, quantity, quality, &dataList);
    }

}

void World::kill_command(Player *cp) {
    if (!cp->hasGMRight(gmr_reload)) {
        return;
    }

    uint32_t counter = 0;

    for (const auto &monster : Monsters) {
        monster->remove();
        ++counter;
    }

    Logger::info(LogFacility::Admin) << cp->to_string() << " nukes " << counter << " monsters" << Log::end;
}

void World::reload_command(Player *cp) {
    if (cp->hasGMRight(gmr_reload)) {
        Logger::info(LogFacility::Admin) << cp->to_string() << " issues a full reload" << Log::end;

        if (reload_tables(cp)) {
            cp->inform("DB tables loaded successfully!");
        } else {
            cp->inform("CRITICAL ERROR: Failure while loading DB tables!");
        }
    }
}

void World::broadcast_command(Player *cp,const std::string &message) {
    if (cp->hasGMRight(gmr_broadcast)) {
#ifdef LOG_TALK
        Logger::info(LogFacility::Player) << *cp << " broadcasts: " << message << Log::end;
#endif
        sendMessageToAllPlayers(message);
    }
}

void World::kickall_command(Player *cp) {
    if (cp->hasGMRight(gmr_forcelogout)) {
        Logger::info(LogFacility::Admin) << cp->to_string() << " kicks all players" << Log::end;
        forceLogoutOfAllPlayers();
    }
}

void World::kickplayer_command(Player *cp, const std::string &player) {
    if (cp->hasGMRight(gmr_forcelogout)) {
        Logger::info(LogFacility::Admin) << cp->to_string() << " kicks " << player << Log::end;
        forceLogoutOfPlayer(player);
    }
}

void World::showIPS_Command(Player *cp) {
    if (cp->hasGMRight(gmr_basiccommands)) {
        Logger::info(LogFacility::Admin) << cp->to_string() << " requests player info" << Log::end;
        sendAdminAllPlayerData(cp);
    }
}

void World::jumpto_command(Player *cp,const std::string &player) {
#ifndef TESTSERVER

    if (cp->hasGMRight(gmr_warp))
#endif
    {
        cp->closeAllShowcasesOfMapContainers();
        teleportPlayerToOther(cp, player);
        Logger::info(LogFacility::Admin) << cp->to_string() << " jumps to player " << player
                                         << " at " << cp->pos.toString() << Log::end;
    }
}

void World::save_command(Player *cp) {
    if (!cp->hasGMRight(gmr_save)) {
        return;
    }

    Logger::info(LogFacility::Admin) << cp->to_string() << " saves all maps" << Log::end;

    Field *tempf;

    for (const auto &player : Players) {
        if (GetPToCFieldAt(tempf, player->pos.x, player->pos.y, player->pos.z)) {
            tempf->SetPlayerOnField(false);
        }
    }

    std::cout << "Save maps" << std::endl;
    Save("Illarion");

    for (const auto &player : Players) {
        if (GetPToCFieldAt(tempf, player->pos.x, player->pos.y, player->pos.z)) {
            tempf->SetPlayerOnField(true);
        }
    }

    std::string tmessage = "*** Maps saved! ***";
    cp->inform(tmessage);
}

void World::talkto_command(Player *cp, const std::string &ts) {
    if (!cp->hasGMRight(gmr_basiccommands)) {
        return;    //quit if the player hasn't the right
    }

    char *tokenize = new char[ ts.length() + 1 ]; //Neuen char mit gr�e des strings anlegen
    strcpy(tokenize, ts.c_str());   //Copy ts to tokenize
    std::cout<<"Tokenizing "<<tokenize<<std::endl;
    char *token;

    if ((token = strtok(tokenize, ","))) {
        std::string player = token;
        delete[] tokenize;

        if ((token = strtok(NULL, "\\"))) {
            std::string message = token;
            Player *tempPl = Players.find(player);

            if (tempPl) {
#ifdef LOG_TALK
                Logger::info(LogFacility::Player) << *cp << " talks to " << *tempPl << ": " << message << Log::end;
#endif
                tempPl->inform(message, Player::informGM);
                return;
            } else {
                TYPE_OF_CHARACTER_ID tid;

                std::stringstream ss;
                ss.str(player);
                ss >> tid;

                tempPl = Players.findID(tid);

                if (tempPl) {
#ifdef LOG_TALK
                    Logger::info(LogFacility::Player) << *cp << " talks to " << *tempPl << ": " << message << Log::end;
#endif
                    tempPl->inform(message, Player::informGM);
                    return;
                }
            }
        }
    } else {
        delete[] tokenize;
    }
}

void World::makeInvisible(Player *cp) {
    if (!cp->hasGMRight(gmr_visible)) {
        return;
    }

    cp->isinvisible = true;
    Logger::info(LogFacility::Admin) << cp->to_string() << " becomes invisible" << Log::end;
    sendRemoveCharToVisiblePlayers(cp->getId(), cp->pos);
}

void World::makeVisible(Player *cp) {
    if (!cp->hasGMRight(gmr_visible)) {
        return;
    }

    cp->isinvisible = false;

    Logger::info(LogFacility::Admin) << cp->to_string() << " becomes visible" << Log::end;

    for (const auto &player : Players.findAllCharactersInScreen(cp->pos)) {
        if (cp != player) {
            ServerCommandPointer cmd(new MoveAckTC(cp->getId(), cp->pos, PUSH, 0));
            player->Connection->addCommand(cmd);
        }
    }

    ServerCommandPointer cmd(new AppearanceTC(cp, cp));
    cp->Connection->addCommand(cmd);
}

void World::ForceIntroduce(Player *cp, const std::string &ts) {
    if (!cp->hasGMRight(gmr_basiccommands)) {
        return;
    }

    Player *tempPl;
    tempPl = Players.find(ts);

    if (tempPl != NULL) {
        forceIntroducePlayer(tempPl, cp);
    } else {
        TYPE_OF_CHARACTER_ID tid;

        // convert arg to digit and try again...
        std::stringstream ss;
        ss.str(ts);
        ss >> tid;

        if (tid) {
            Player *player = Players.findID(tid);

            if (player) {
                forceIntroducePlayer(player, cp);
            }
        }
    }
}

void World::ForceIntroduceAll(Player *cp) {
    if (!cp->hasGMRight(gmr_basiccommands)) {
        return;
    }

    for (const auto &player : Players.findAllCharactersInRangeOf(cp->pos, cp->getScreenRange())) {
        if (cp != player) {
            forceIntroducePlayer(player, cp);
        }
    }
}

void World::teleportPlayerToOther(Player *cp, std::string ts) {
    if (!cp->hasGMRight(gmr_warp)) {
        return;
    }

    Player *tempPl;
    tempPl = Players.find(ts);

    if (tempPl) {
        cp->Warp(tempPl->pos);
    } else {
        TYPE_OF_CHARACTER_ID tid;

        // convert arg to digit and try again...
        std::stringstream ss;
        ss.str(ts);
        ss >> tid;

        if (tid) {
            tempPl = Players.findID(tid);

            if (tempPl) {
                cp->Warp(tempPl->pos);
            }
        }
    }
}


void World::forceLogoutOfAllPlayers() {
    Field *tempf;

    for (const auto &player : Players) {
        if (GetPToCFieldAt(tempf, player->pos.x, player->pos.y, player->pos.z)) {
            tempf->SetPlayerOnField(false);
        }

        Logger::info(LogFacility::Admin) << "--- kicked: " << *player << Log::end;
        ServerCommandPointer cmd(new LogOutTC(SERVERSHUTDOWN));
        player->Connection->shutdownSend(cmd);
        PlayerManager::get()->getLogOutPlayers().non_block_push_back(player);
    }

    Players.clear();
}


bool World::forceLogoutOfPlayer(const std::string &name) {
    Player *temp = Players.find(name);

    if (temp) {
        Logger::info(LogFacility::Admin) << "--- kicked: " << temp->to_string() << Log::end;
        ServerCommandPointer cmd(new LogOutTC(BYGAMEMASTER));
        temp->Connection->shutdownSend(cmd);
        return true;
    } else {
        return false;
    }
}


void World::sendAdminAllPlayerData(Player *&admin) {
    if (!admin->hasGMRight(gmr_basiccommands)) {
        return;
    }

    ServerCommandPointer cmd(new AdminViewPlayersTC());
    admin->Connection->addCommand(cmd);

}


// !warp_to X<,| >Y[<,| >Z] || !warp_to Z
void World::warpto_command(Player *cp, const std::string &ts) {
#ifndef TESTSERVER

    if (!cp->hasGMRight(gmr_warp)) {
        return;
    }

#endif

    position warpto;
    char *tokenize = new char[ ts.length() + 1 ];

    strcpy(tokenize, ts.c_str());
    std::cout << "Tokenizing " << tokenize << std::endl;
    char *thistoken;

    if ((thistoken = strtok(tokenize, " ,"))) {
        if (ReadField(thistoken, warpto.x)) {
            if ((thistoken = strtok(NULL, " ,"))) {
                if (ReadField(thistoken, warpto.y)) {
                    if ((thistoken = strtok(NULL, " ,"))) {
                        if (ReadField(thistoken, warpto.z)) {
                            //warpPlayer( cp, warpto );
                            cp->forceWarp(warpto);
                        }
                    }
                    // Must give X and Y, but not Z
                    else {
                        warpto.z = cp->pos.z;
                        cp->forceWarp(warpto);
                        //warpPlayer( cp, warpto );
                    }
                }
            }
            // Enable !warp_to Z for easy level change
            else {
                warpto.z = warpto.x;
                warpto.x = cp->pos.x;
                warpto.y = cp->pos.y;
                cp->forceWarp(warpto);
                //warpPlayer( cp, warpto );
            }
        }
    }

    Logger::info(LogFacility::Admin) << cp->to_string() << " warps to " << warpto.toString() << Log::end;

    delete [] tokenize;
}


// !summon <player>
void World::summon_command(Player *cp, const std::string &tplayer) {
    if (!cp->hasGMRight(gmr_summon)) {
        return;
    }

    Player *tempPl;
    tempPl = Players.find(tplayer);

    if (tempPl != NULL) {
        Logger::info(LogFacility::Admin) << cp->to_string() << " summons player " << tempPl->to_string() << " to " << cp->pos.toString() << Log::end;
        tempPl->Warp(cp->pos);
    } else {
        TYPE_OF_CHARACTER_ID tid;

        // convert arg to digit and try again...
        std::stringstream ss;
        ss.str(tplayer);
        ss >> tid;

        if (tid) {
            tempPl = Players.findID(tid);

            if (tempPl) {
                Logger::info(LogFacility::Admin) << *cp << " summons player " << *tempPl << " to " << cp->pos.toString() << Log::end;
                tempPl->Warp(cp->pos);
            }
        }
    }

}


// !ban <time> [m|h|d] <player>
void World::ban_command(Player *cp, const std::string &timeplayer) {
    if (!cp->hasGMRight(gmr_ban)) {
        return;
    }

    char *tokenize = new char[ timeplayer.length() + 1 ];

    strcpy(tokenize, timeplayer.c_str());

    char *thistoken;

    if ((thistoken = strtok(tokenize, " ")) != NULL) {
        short int jailtime = 0;

        if (ReadField(thistoken, jailtime)) {
            char *tcharp = strtok(NULL, " ");

            if (tcharp != NULL) {
                int multiplier = 0;

                std::string tplayer;
                std::string timescale = tcharp;

                if (timescale == "m") {
                    multiplier = 60;
                    timescale = "";
                } else if (timescale == "h") {
                    multiplier = 3600;
                    timescale = "";
                } else if (timescale == "d") {
                    multiplier = 86400;
                    timescale = "";
                }

                char *tcharp = strtok(NULL, "\\");

                if (tcharp != NULL) {
                    tplayer = tcharp;

                    if (timescale != "") {
                        tplayer = timescale + " " + tplayer;
                    }
                } else {
                    tplayer = timescale;
                    timescale = "d";
                    multiplier = 86400;
                }

                Player *tempPl;
                tempPl = Players.find(tplayer);

                if (!tempPl) {
                    TYPE_OF_CHARACTER_ID tid;

                    // convert arg to digit and try again...
                    std::stringstream ss;
                    ss.str(tplayer);
                    ss >> tid;

                    if (tid) {
                        tempPl = Players.findID(tid);
                    }
                }

                if (tempPl) {

                    ban(tempPl, jailtime * multiplier, cp->getId());

                    Logger::info(LogFacility::Admin) << cp->to_string() << " bans player " << tempPl->to_string() << " for " << jailtime << timescale << Log::end;
                    std::string tmessage = "*** Banned " + tempPl->to_string();
                    cp->inform(tmessage);

                } else {
                    std::string tmessage = "*** Could not find " + tplayer;
                    std::cout << tmessage << std::endl;
                    cp->inform(tmessage);
                }
            }
        }
    }

    delete [] tokenize;

}

void World::banbyname(Player *cp, short int banhours, const std::string &tplayer) {
    if (!cp->hasGMRight(gmr_ban)) {
        return;
    }

    Player *tempPl;
    tempPl = Players.find(tplayer);

    if (tempPl != NULL) {

        ban(tempPl, static_cast<int>(banhours * 3600), cp->getId());

        Logger::info(LogFacility::Admin) << cp->to_string() << " bans player " << tempPl->to_string() << " for " << banhours << "h" << Log::end;
        std::string tmessage = "*** Banned " + tempPl->to_string();
        cp->inform(tmessage);

    } else {
        std::string tmessage = "*** Could not find " + tplayer;
        std::cout << tmessage << std::endl;
        cp->inform(tmessage);
    }

}

void World::banbynumber(Player *cp, short int banhours, TYPE_OF_CHARACTER_ID tid) {
    if (!cp->hasGMRight(gmr_ban)) {
        return;
    }

    Player *tempPl = Players.findID(tid);

    if (tempPl) {

        ban(tempPl, static_cast<int>(banhours * 3600), cp->getId());

        Logger::info(LogFacility::Admin) << *cp << " bans player " << *tempPl << " for " << banhours << "h" << Log::end;
        std::string tmessage = "*** Banned " + tempPl->to_string();
        cp->inform(tmessage);

    } else {
        std::string tmessage = "*** Could not find " + std::to_string(tid);
        std::cout << tmessage << std::endl;
        cp->inform(tmessage);
    }


}


void World::ban(Player *cp, int bantime, TYPE_OF_CHARACTER_ID gmid) {
    if (bantime >= 0) {
        if (bantime > 0) {
            cp->SetStatus(BANNEDFORTIME);
            time_t ttime;
            time(&ttime);
            // Banned for seconds
            cp->SetStatusTime(ttime + bantime);
            cp->SetStatusGM(gmid);
        } else if (bantime == 0) {
            cp->SetStatus(BANNED);
            cp->SetStatusTime(0);
            cp->SetStatusGM(gmid);
        }

        forceLogoutOfPlayer(cp->getName());

    }

}


// !who [player]
void World::who_command(Player *cp, const std::string &tplayer) {
#ifndef TESTSERVER

    if (!cp->hasGMRight(gmr_basiccommands)) {
        return;
    }

#endif

    if (tplayer == "") {

        std::string tmessage = "";

        for (const auto &p : Players) {
            if (tmessage.length() > 0) {
                tmessage = tmessage + ", ";
            }

            tmessage = tmessage + p->to_string();
        }

        if (tmessage.length() > 0) {
            cp->inform(tmessage);
        }
    } else {

        Player *tempPl = Players.find(tplayer);

        if (!tempPl) {
            TYPE_OF_CHARACTER_ID tid;

            // convert arg to digit and try again...
            std::stringstream ss;
            ss.str(tplayer);
            ss >> tid;

            if (tid) {
                tempPl = Players.findID(tid);
            }
        }

        if (tempPl) {
            std::string tmessage = tempPl->to_string();

            tmessage = tmessage + " x" + stream_convert<std::string>(tempPl->pos.x);
            tmessage = tmessage + " y" + stream_convert<std::string>(tempPl->pos.y);
            tmessage = tmessage + " z" + stream_convert<std::string>(tempPl->pos.z);
            tmessage = tmessage + " HPs:" + stream_convert<std::string>(tempPl->getAttribute(Character::hitpoints));
            tmessage = tmessage + ((tempPl->IsAlive()) ? " Alive" : " Dead");
            tmessage = tmessage + " Mental Capacity: " + stream_convert<std::string>(tempPl->getMentalCapacity());
            std::string german = " German";
            std::string english = " English";
            tmessage = tmessage + tempPl->nls(german, english);

            cp->inform(tmessage);
        }
    }
}


void World::tile_command(Player *cp, const std::string &ttilenumber) {
    if (!cp->hasGMRight(gmr_settiles)) {
        return;
    }

    short int tilenumber = 0;

    if (ReadField(ttilenumber.c_str(), tilenumber)) {
        setNextTile(cp, tilenumber);
    }

}


void World::setNextTile(Player *cp, unsigned char tilenumber) {

    position tpos = cp->getFrontalPosition();

    Field *tempf;

    if (GetPToCFieldAt(tempf, tpos.x, tpos.y, tpos.z)) {
        tempf->setTileId(tilenumber);
        tempf->updateFlags();
    }

    //update the current area
    cp->sendRelativeArea(0);
    sendAllVisibleCharactersToPlayer(cp, true);

}


void World::turtleon_command(Player *cp, const std::string &ttilenumber) {
    if (!cp->hasGMRight(gmr_settiles)) {
        return;
    }

    short int tilenumber = 0;

    if (ReadField(ttilenumber.c_str(), tilenumber)) {
        cp->setTurtleActive(true);
        cp->setTurtleTile(tilenumber);
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

    Logger::info(LogFacility::Admin) << cp->to_string() << " turns on clipping" << Log::end;
    cp->setClippingActive(true);
}


void World::clippingoff_command(Player *cp) {
    if (!cp->hasGMRight(gmr_clipping)) {
        return;
    }

    Logger::info(LogFacility::Admin) << cp->to_string() << " turns off clipping" << Log::end;
    cp->setClippingActive(false);
}


void World::what_command(Player *cp) {
    position front = cp->getFrontalPosition();

    cp->inform("Facing:");
    std::stringstream message;

    message << "- Position " << front;
    cp->inform(message.str());
    Field *tempf;

    if (GetPToCFieldAt(tempf, front)) {
        message.str("");

        message << "- Tile " << tempf->getTileId();
        cp->inform(message.str());
        Item top;

        if (tempf->ViewTopItem(top)) {
            message.str("");

            message << "- Item " << top.getId();

#ifndef TESTSERVER

            if (cp->hasGMRight(gmr_basiccommands))
#endif
            {
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

        if (character != 0) {
            message.str("");
            uint32_t id = character->getId();

            if (id >= DYNNPC_BASE) {
                message << "- Dynamic NPC";
            } else if (id >= NPC_BASE) {
                message << "- NPC " << id-NPC_BASE;
            } else if (id >= MONSTER_BASE) {
                message << "- Monster " << dynamic_cast<Monster *>(character)->getType();
            } else {
                message << "- Player";
            }

            cp->inform(message.str());
        }
    }
}


void World::playersave_command(Player *cp) {
    if (!cp->hasGMRight(gmr_save)) {
        return;
    }

    Logger::info(LogFacility::Admin) << *cp << " saves all players" << Log::end;

    for (const auto &p : Players) {
        p->save();
    }

    std::string tmessage = "*** All online players saved! ***";
    cp->inform(tmessage);

}


// !teleport X<,| >Y[<,| >Z]
void World::teleport_command(Player *cp, const std::string &ts) {

    if (!cp->hasGMRight(gmr_warp)) {
        return;
    }

    position teleportto;
    char *tokenize = new char[ ts.length() + 1 ];

    strcpy(tokenize, ts.c_str());
    std::cout << "Tokenizing " << tokenize << std::endl;
    char *thistoken;

    if ((thistoken = strtok(tokenize, " ,")) != NULL) {
        if (ReadField(thistoken, teleportto.x)) {
            if ((thistoken = strtok(NULL, " ,")) != NULL) {
                if (ReadField(thistoken, teleportto.y)) {
                    if ((thistoken = strtok(NULL, " ,")) != NULL) {
                        if (ReadField(thistoken, teleportto.z)) {
                            if (addWarpField(cp->pos, teleportto, 0, 0)) {
                                std::string tmessage = "*** Warp Field Added! ***";
                                cp->inform(tmessage);
                            } else {
                                std::string tmessage = "*** Warp Field *NOT* Added! ***";
                                cp->inform(tmessage);
                            };
                        }
                    }
                }
            }
        }
    }

    delete [] tokenize;
}


void World::gmhelp_command(Player *cp) {
    if (!cp->hasGMRight(gmr_basiccommands)) {
        return;
    }

    std::string tmessage = " <> - parameter.  [] - optional.  | = choice.  () = shortcut";
    cp->inform(tmessage);

    if (cp->hasGMRight(gmr_basiccommands)) {
        tmessage = "!what - sends different informations of the field or the character in front of you.";
        cp->inform(tmessage);
        tmessage = "!who [player] - List all players online or a single player if specified.";
        cp->inform(tmessage);
        tmessage = "!forceintroduce <char id|char name> - (!fi) introduces the char to all gms in range.";
        cp->inform(tmessage);
        tmessage = "!forceintroduceall - (!fia) introduces all chars in sight to you.";
        cp->inform(tmessage);
        tmessage = "!talkto <playername|id>, <message> - (!tt) sends a message to a specific player important is the , after the id or name!";
        cp->inform(tmessage);
        tmessage = "!broadcast <message> - (!bc) Broadcasts the message <message> to all players IG.";
        cp->inform(tmessage);
        tmessage = "!create id [quantity [quality [[data_key=data_value] ...]]] creates an item in your inventory.";

    }

    if (cp->hasGMRight(gmr_warp)) {
        tmessage = "!warp <x> <y> [z] - (!w) to x, y, z location.";
        cp->inform(tmessage);
        tmessage = "!add_teleport <x> <y> <z> - Adds a teleportfield in front of you to the field <x> <y> <z>.";
        cp->inform(tmessage);
        tmessage = "!showwarpfields <range> - Shows all warpfields in the range <range>.";
        cp->inform(tmessage);
        tmessage = "!removewarpfield <x> <y> <z> - Removes the warpfield at the position <x> <y> <z>.";
        cp->inform(tmessage);
        tmessage = "!jumpto <playerid|name> - (!j) teleports you to the player.";
        cp->inform(tmessage);

    }

    if (cp->hasGMRight(gmr_summon)) {
        tmessage = "!summon <player> - (!s) Summons a player to you.";
        cp->inform(tmessage);
    }

    if (cp->hasGMRight(gmr_ban)) {
        tmessage = "!ban <time> [m|h|d] <player> - (!b) Bans the player <player> for <time> [m]inutes/[h]ours/[d]ays.";
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
        tmessage = "!nologin <true|false> - changes the login state, with true only gm's can log in.";
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
        tmessage = "!kick <playerid> - Kicks the player with the id out of the game.";
        cp->inform(tmessage);
    }
}

//! parse GMCommands of the form !<string1> <string2> and process them
bool World::parseGMCommands(Player *cp, const std::string &text) {

    // did we find a command?
    bool done = false;

    // use a regexp to match for commands...
    regex_t expression;
    regcomp(&expression, "^!([^ ]+) ?(.*)?$",REG_ICASE|REG_EXTENDED);
    regmatch_t matches[3];

    if (regexec(&expression, text.c_str(), 3, matches, 0) == 0) {
        // we found something...
        CommandIterator it = GMCommands.find(text.substr(matches[1].rm_so, matches[1].rm_eo-1));

        // do we have a matching command?
        if (it != GMCommands.end()) {
            if (matches[2].rm_so != -1) { // !bla something
                (it->second)(this, cp, text.substr(matches[2].rm_so));
            } else { // !bla
                (it->second)(this, cp, "");
            }

            done = true;
        }
    }

    regfree(&expression);

    return done;

}

extern MonsterTable *MonsterDescriptions;

void reportError(Player *cp, std::string msg) {
    std::cerr << "ERROR: " << msg << std::endl;
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

    sendMessageToAllPlayers("### The server is reloading, this may cause some lag ###");

    bool ok = Data::Skills.reloadBuffer();

    if (ok) {
        Data::Skills.activateBuffer();
    }

    MonsterTable *MonsterDescriptions_temp = 0;
    ScheduledScriptsTable *ScheduledScripts_temp = 0;

    if (ok) {
        QuestNodeTable::getInstance()->reload();
    }

    if (ok) {
        ok = Data::reload();
    }

    if (ok) {
        MonsterDescriptions_temp = new MonsterTable();

        if (MonsterDescriptions_temp == NULL || !MonsterDescriptions_temp->dataOK()) {
            reportTableError(cp, "monster");
            ok = false;
        }
    }

    if (ok) {
        std::cerr << "Attempting to reload Scheduler" << std::endl;
        ScheduledScripts_temp = new ScheduledScriptsTable();
        std::cerr << "Created new Scheduler" << std::endl;

        if (ScheduledScripts_temp == NULL || !ScheduledScripts_temp->dataOK()) {
            reportTableError(cp, "scheduledscripts");
            ok = false;
        }
    }

    if (!ok) {
        if (MonsterDescriptions_temp != NULL) {
            delete MonsterDescriptions_temp;
        }

        if (ScheduledScripts_temp != NULL) {
            delete ScheduledScripts_temp;
        }
    } else {
        // if everything went well, delete old tables and set up new tables
        //Mutex für login logout sperren so das aktuell keiner mehr einloggen kann
        PlayerManager::get()->setLoginLogout(true);
        delete MonsterDescriptions;
        MonsterDescriptions = MonsterDescriptions_temp;
        delete scheduledScripts;
        scheduledScripts = ScheduledScripts_temp;
        //Mutex entsperren.
        PlayerManager::get()->setLoginLogout(false);

        //Reload the standard Fighting script
        try {
            std::shared_ptr<LuaWeaponScript> tmpScript(new LuaWeaponScript("server.standardfighting"));
            standardFightingScript = tmpScript;
        } catch (ScriptException &e) {
            reportScriptError(cp, "standardfighting", e.what());
            ok = false;
        }

        try {
            std::shared_ptr<LuaLookAtPlayerScript>tmpScript(new LuaLookAtPlayerScript("server.playerlookat"));
            lookAtPlayerScript = tmpScript;
        } catch (ScriptException &e) {
            reportScriptError(cp, "playerlookat", e.what());
            ok = false;
        }

        try {
            std::shared_ptr<LuaLookAtItemScript>tmpScript(new LuaLookAtItemScript("server.itemlookat"));
            lookAtItemScript = tmpScript;
        } catch (ScriptException &e) {
            reportScriptError(cp, "itemlookat", e.what());
            ok = false;
        }

        try {
            std::shared_ptr<LuaPlayerDeathScript>tmpScript(new LuaPlayerDeathScript("server.playerdeath"));
            playerDeathScript = tmpScript;
        } catch (ScriptException &e) {
            reportScriptError(cp, "playerdeath", e.what());
            ok = false;
        }

        try {
            std::shared_ptr<LuaLoginScript>tmpScript(new LuaLoginScript("server.login"));
            loginScript = tmpScript;
        } catch (ScriptException &e) {
            reportScriptError(cp, "login", e.what());
            ok = false;
        }

        try {
            std::shared_ptr<LuaLogoutScript>tmpScript(new LuaLogoutScript("server.logout"));
            logoutScript = tmpScript;
        } catch (ScriptException &e) {
            reportScriptError(cp, "logout", e.what());
            ok = false;
        }

        try {
            std::shared_ptr<LuaLearnScript>tmpScript(new LuaLearnScript("server.learn"));
            learnScript = tmpScript;
        } catch (ScriptException &e) {
            reportScriptError(cp, "learn", e.what());
            ok = false;
        }

        try {
            std::shared_ptr<LuaDepotScript>tmpScript(new LuaDepotScript("server.depot"));
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

        for (const auto &p : Players) {
            p->sendCompleteQuestProgress();
        }

        try {
            std::shared_ptr<LuaReloadScript> tmpScript(new LuaReloadScript("server.reload"));
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

    Logger::info(LogFacility::Admin) << player->to_string() << " sets spawn to " << enable << Log::end;

    world->enableSpawn(enable);
}


// create a new area starting at x,y,z with dimension w,h, filltile ft (create_area x y z w h ft)
void create_area_command(World *world, Player *player,const std::string &params) {
    if (!player->hasGMRight(gmr_import)) {
        return;
    }

    std::stringstream ss(params);
    int x,y,z,w,h, filltile;
    x=y=z=w=h=filltile=-65535;
    ss >> x;
    ss >> y;
    ss >> z;
    ss >> w;
    ss >> h;
    ss >> filltile;

    if (x==-65535 || y == -65535 || z == -65535 || w < 1 || h < 1 || filltile < 0) {
        std::cout << "error in create_area_command issued by " << player->to_string() << "!" << std::endl;
        std::cout << "positions: " << x << "\t" << y << '\t' << z << '\t' << w << '\t' << h << '\t' << std::endl;
        return;
    }

    WorldMap::map_t tempmap(new Map(w,h));
    tempmap->Init(x, y, z);

    Field *tempf;

    for (int _x=0; _x<w; ++_x)
        for (int _y=0; _y<h; ++_y) {
            if (tempmap->GetPToCFieldAt(tempf, _x+x, _y+y)) {
                tempf->setTileId(filltile);
                tempf->updateFlags();
            } else {
                std::cerr << "error in create map: " << x << " " << y << " " << z << " " << _x << " " << _y << " " << filltile << std::endl;
            }

        }

    world->maps.InsertMap(tempmap);

    std::string tmessage = "map inserted.";
    player->inform(tmessage);
    std::cerr << "Map created by " << player->to_string() << " on " << x << " - " << y << " - " << z << " with w: " << w << " h: " << h << "ft: " << filltile << std::endl;

}

void set_login(World *world, Player *player, const std::string &st) {
    if (!player->hasGMRight(gmr_loginstate)) {
        return;
    }

    bool enable = true;

    if (st == "true") {
        enable = false;
    }

    world->allowLogin(enable);
    Logger::info(LogFacility::Admin) << player->to_string() << " set allowLogin to " << enable << Log::end;
    std::string tmessage = "nologin set to: ";
    tmessage += enable ? "false" : "true";
    player->inform(tmessage);
}

bool World::exportMaps(Player *cp) {
    if (!cp->hasGMRight(gmr_import)) {
        return false;
    }

    std::string exportDir = directory + std::string(MAPDIR) + "export/";
    return maps.exportTo(exportDir);
}

void World::removeTeleporter(Player *cp, const std::string &ts) {
    if (!cp->hasGMRight(gmr_warpfields)) {
        return;
    }

    position teleport;
    char *tokenize = new char[ ts.length() + 1 ];

    strcpy(tokenize, ts.c_str());
    std::cout << "Tokenizing " << tokenize << std::endl;
    char *thistoken;

    if ((thistoken = strtok(tokenize, " ,")) != NULL) {
        if (ReadField(thistoken, teleport.x)) {
            if ((thistoken = strtok(NULL, " ,")) != NULL) {
                if (ReadField(thistoken, teleport.y)) {
                    if ((thistoken = strtok(NULL, " ,")) != NULL) {
                        if (ReadField(thistoken, teleport.z)) {
                            if (removeWarpField(teleport)) {
                                std::string tmessage = "*** Warp Field deleted! ***";
                                cp->inform(tmessage);
                            } else {
                                std::string tmessage = "*** Warp Field *NOT* deleted! ***";
                                cp->inform(tmessage);
                            };
                        }
                    }
                }
            }
        }
    }

    delete [] tokenize;
}

void World::showWarpFieldsInRange(Player *cp, const std::string &ts) {
    if (!cp->hasGMRight(gmr_warpfields)) {
        return;
    }

    short int range = 0;

    if (ReadField(ts.c_str(), range)) {
        std::vector<position> warpfieldsinrange;

        if (findWarpFieldsInRange(cp->pos, range, warpfieldsinrange)) {
            std::string message;
            cp->inform("Start list of warpfields:");

            for (const auto &warpfield : warpfieldsinrange) {
                position target;
                GetField(warpfield)->GetWarpField(target);
                message = "Warpfield at " + warpfield.toString() + " to " + target.toString();
                cp->inform(message);
            }

            cp->inform("End list of warpfields.");
        }
    }
}
