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

#include <sstream>
#include <regex.h>

#include "Player.hpp"
#include "MonitoringClients.hpp"

#include "netinterface/protocol/ServerCommands.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"

#include "db/InsertQuery.hpp"

#include "version.hpp"

template< typename To, typename From> To stream_convert(const From &from) {
    std::stringstream stream;
    stream << from;
    To to;
    stream >> to;
    return to;
}

// register any Player commands here...
void World::InitPlayerCommands() {

    PlayerCommands["gm"] = [](World *world, Player *player, const std::string &text) -> bool { return world->gmpage_command(player, text); };
    PlayerCommands["name"] = [](World *world, Player *player, const std::string &text) -> bool { world->name_command(player, text); return true; };
    PlayerCommands["language"] = [](World *world, Player *player, const std::string &text) -> bool { return world->active_language_command(player, text); };
    PlayerCommands["l"] = PlayerCommands["language"];
    PlayerCommands["version"] = [](World *world, Player *player, const std::string &) -> bool { world->version_command(player); return true; };
    PlayerCommands["v"] = PlayerCommands["version"];
}


//! parse PlayerCommands of the form !<string1> <string2> and process them
bool World::parsePlayerCommands(Player *cp, const std::string &text) {

    // did we find a command?
    bool done = false;

    // use a regexp to match for commands...
    regex_t expression;
    regcomp(&expression, "^!([^ ]+) ?(.*)?$",REG_ICASE|REG_EXTENDED);
    regmatch_t matches[3];

    if (regexec(&expression, text.c_str(), 3, matches, 0) == 0) {
        // we found something...
        CommandIterator it = PlayerCommands.find(text.substr(matches[1].rm_so, matches[1].rm_eo-1));

        // do we have a matching command?
        if (it != PlayerCommands.end()) {
            if (matches[2].rm_so != -1) { // !bla something
                done = (it->second)(this, cp, text.substr(matches[2].rm_so));
            } else { // !bla
                done = (it->second)(this, cp, "");
            }
        }
    }

    regfree(&expression);

    return done;

}
void World::name_command(Player *cp, const std::string &ts) {
    char *tokenize = new char[ ts.length() + 1 ];
    TYPE_OF_CHARACTER_ID player;
    std::string name;
    strcpy(tokenize, ts.c_str());
    std::cout << "Tokenizing " << tokenize << std::endl;
    char *thistoken;

    if ((thistoken = strtok(tokenize, " ,"))) {
        // convert arg to digit and try again...
        std::stringstream ss;
        ss.str(thistoken);
        ss >> player;

        if (player) {
            if ((thistoken = strtok(nullptr, ""))) {
                Players.for_each([&](Player *p) {
                    if (p->getId() == player) {
                        std::string newname(thistoken);
                        name = "! " + newname;
                        ServerCommandPointer cmd = std::make_shared<IntroduceTC>(player, name);
                        cp->Connection->addCommand(cmd);
                    }
                });
            }

        }

    }

    delete [] tokenize;
}

// GM page (!gm <text>)
bool World::gmpage_command(Player *player, const std::string &ticket) {
    try {
        logGMTicket(player, ticket, false);
        player->inform("--- The message has been delivered to the GM team. ---");
        return true;
    } catch (...) {
    }

    return false;
}

void World::logGMTicket(Player *player, const std::string &ticket, bool automatic) {
    using namespace Database;

    InsertQuery insQuery;
    insQuery.setServerTable("gmpager");
    const InsertQuery::columnIndex userColumn = insQuery.addColumn("pager_user");
    const InsertQuery::columnIndex textColumn = insQuery.addColumn("pager_text");
    insQuery.addValue(userColumn, player->getId());
    insQuery.addValue(textColumn, ticket);

    insQuery.execute();

    std::string message;

    if (automatic) {
        message = "Automatic page about " + player->to_string() + ": " + ticket;
    } else {
        message = "Page from " + player->to_string() + ": " + ticket;
    }

    sendMessageToAdmin(message);
    ServerCommandPointer cmd = std::make_shared<BBMessageTC>(message,2);
    monitoringClientList->sendCommand(cmd);
}

// !language <language>, language=common, human, dwarfen, elven, lizard, orc, ...
bool World::active_language_command(Player *cp, const std::string &language) {
    if (strcmp(language.c_str(),"common")==0) {
        cp->setActiveLanguage(0);
    }

    if (strcmp(language.c_str(),"human")==0) {
        cp->setActiveLanguage(1);
    }

    if (strcmp(language.c_str(),"dwarf")==0) {
        cp->setActiveLanguage(2);
    }

    if (strcmp(language.c_str(),"elf")==0) {
        cp->setActiveLanguage(3);
    }

    if (strcmp(language.c_str(),"lizard")==0) {
        cp->setActiveLanguage(4);
    }

    if (strcmp(language.c_str(),"orc")==0) {
        cp->setActiveLanguage(5);
    }

    if (strcmp(language.c_str(),"halfling")==0) {
        cp->setActiveLanguage(6);
    }

    if (strcmp(language.c_str(),"fairy")==0) {
        cp->setActiveLanguage(7);
    }

    if (strcmp(language.c_str(),"gnome")==0) {
        cp->setActiveLanguage(8);
    }

    if (strcmp(language.c_str(),"goblin")==0) {
        cp->setActiveLanguage(9);
    }

    if (strcmp(language.c_str(),"ancient")==0) {
        cp->setActiveLanguage(10);
    }

    return true;
}

void World::version_command(Player *player) {
    player->inform("Version: " SERVER_VERSION);
}

