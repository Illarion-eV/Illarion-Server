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

#include "netinterface/protocol/ServerCommands.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"

#include "db/InsertQuery.hpp"


template< typename To, typename From> To stream_convert(const From &from) {
    std::stringstream stream;
    stream << from;
    To to;
    stream >> to;
    return to;
}

// register any Player commands here...
void World::InitPlayerCommands() {

    PlayerCommands["prefix"] = [](World *world, Player *player, const std::string &text) -> bool { return world->prefix_command(player, text); };
    PlayerCommands["suffix"] = [](World *world, Player *player, const std::string &text) -> bool { return world->suffix_command(player, text); };
    PlayerCommands["gm"] = [](World *world, Player *player, const std::string &text) -> bool { return world->gmpage_command(player, text); };
    PlayerCommands["name"] = [](World *world, Player *player, const std::string &text) -> bool { world->name_command(player, text); return true; };
    PlayerCommands["language"] = [](World *world, Player *player, const std::string &text) -> bool { return world->active_language_command(player, text); };
    PlayerCommands["l"] = PlayerCommands["language"];
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

    if ((thistoken = strtok(tokenize, " ,")) != NULL) {
        // convert arg to digit and try again...
        std::stringstream ss;
        ss.str(thistoken);
        ss >> player;

        if (player) {
            if ((thistoken = strtok(NULL, "")) != NULL) {
                std::cout<<"name Command from: " << cp->name << " intr: " << player << " with name: "<< thistoken;
                PLAYERVECTOR::iterator playerIterator;

                for (playerIterator = Players.begin(); playerIterator < Players.end(); ++playerIterator) {
                    if ((*playerIterator)->id == player) {
                        std::string newname(thistoken);
                        name = "! " + newname;
                        boost::shared_ptr<BasicServerCommand>cmd(new IntroduceTC((*playerIterator)->id, name));
                        cp->Connection->addCommand(cmd);
                    }
                }
            }

        }

    }

    delete [] tokenize;
}

// GM page (!gm <text>)
bool World::gmpage_command(Player *player, const std::string &ticket) {
    try {
        logGMTicket(player, ticket, "Page from " + player->name + ": ");
        player->inform("--- The message has been delivered to the GM team. ---");
        return true;
    } catch (...) {
    }

    return false;
}

void World::logGMTicket(Player *player, const std::string &ticket, const std::string &prefix) {
    using namespace Database;

    InsertQuery insQuery;
    insQuery.setServerTable("gmpager");
    const InsertQuery::columnIndex userColumn = insQuery.addColumn("pager_user");
    const InsertQuery::columnIndex textColumn = insQuery.addColumn("pager_text");
    insQuery.addValue(userColumn, player->id);
    insQuery.addValue(textColumn, ticket);

    insQuery.execute();

    std::string message = prefix + ticket;
    sendMessageToAdmin(message);
    boost::shared_ptr<BasicServerCommand>cmd(new BBMessageTC(message,2));
    monitoringClientList->sendCommand(cmd);
}

// !prefix <prefix>
bool World::prefix_command(Player *cp, const std::string &tprefix) {

    cp->prefix = tprefix;

    std::string tstring = "Okay, ";

    if ((cp->prefix != "") && (cp->prefix !="NULL")) {
        tstring = tstring + cp->prefix + std::string(" ");
    }

    tstring = tstring + cp->name;

    if ((cp->suffix != "") && (cp->prefix !="NULL")) {
        tstring = tstring + std::string(" ") + cp->suffix;
    }

    cp->inform(tstring);
    return true;

}


// !suffix <suffix>
bool World::suffix_command(Player *cp, const std::string &tsuffix) {

    cp->suffix = tsuffix;

    std::string tstring = "Okay, ";

    if ((cp->prefix != "") && (cp->prefix != "NULL")) {
        tstring = tstring + cp->prefix + std::string(" ");
    }

    tstring = tstring + cp->name;

    if ((cp->suffix != "") && (cp->prefix !="NULL")) {
        tstring = tstring + std::string(" ") + cp->suffix;
    }

    cp->inform(tstring);
    return true;

}

// !language <language>, language=common, human, dwarfen, elven, lizard, orc, ...
bool World::active_language_command(Player *cp, const std::string &language) {
    if (strcmp(language.c_str(),"common")==0) {
        cp->activeLanguage=0;
    }

    if (strcmp(language.c_str(),"human")==0) {
        cp->activeLanguage=1;
    }

    if (strcmp(language.c_str(),"dwarf")==0) {
        cp->activeLanguage=2;
    }

    if (strcmp(language.c_str(),"elf")==0) {
        cp->activeLanguage=3;
    }

    if (strcmp(language.c_str(),"lizard")==0) {
        cp->activeLanguage=4;
    }

    if (strcmp(language.c_str(),"orc")==0) {
        cp->activeLanguage=5;
    }

    if (strcmp(language.c_str(),"halfling")==0) {
        cp->activeLanguage=6;
    }

    if (strcmp(language.c_str(),"fairy")==0) {
        cp->activeLanguage=7;
    }

    if (strcmp(language.c_str(),"gnome")==0) {
        cp->activeLanguage=8;
    }

    if (strcmp(language.c_str(),"goblin")==0) {
        cp->activeLanguage=9;
    }

    if (strcmp(language.c_str(),"ancient")==0) {
        cp->activeLanguage=10;
    }

    //std::cout << cp->name << "switched language to " << cp->activeLanguage << "with " << language << "\n";

    return true;
}

