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

#include "Command.hpp"


template< typename To, typename From> To stream_convert(const From &from) {
    std::stringstream stream;
    stream << from;
    To to;
    stream >> to;
    return to;
}

// register any Player commands here...
void World::InitPlayerCommands() {

    PlayerCommands["prefix"] = new Command(&World::prefix_command);
    PlayerCommands["suffix"] = new Command(&World::suffix_command);
    PlayerCommands["prison"] = new Command(&World::player_prison_command);
    PlayerCommands["p"] = PlayerCommands["prison"];
    PlayerCommands["gm"] = new Command(&World::gmpage_command);
    PlayerCommands["name"] = new Command(&World::name_command);
    PlayerCommands["language"] = new Command(&World::active_language_command);
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
                done = (*it->second)(this, cp, text.substr(matches[2].rm_so));
            } else { // !bla
                done = (*it->second)(this, cp, "");
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
bool World::gmpage_command(Player *cp, const std::string &ts) {
    //PLAYERVECTOR::iterator pIterator;
    //FILE * f;
    //CPlayer* who;
    std::string tmessage = "Page from " + cp->name + ": " + ts;

    try {
        using namespace Database;
        InsertQuery insQuery;
        insQuery.setServerTable("gmpager");
        const InsertQuery::columnIndex userColumn = insQuery.addColumn("pager_user");
        const InsertQuery::columnIndex textColumn = insQuery.addColumn("pager_text");
        insQuery.addValue(userColumn, cp->id);
        insQuery.addValue(textColumn, ts);

        insQuery.execute();

        sendMessageToAdmin(tmessage);
        boost::shared_ptr<BasicServerCommand>cmd(new BBMessageTC(tmessage,2));
        monitoringClientList->sendCommand(cmd);
        cp->sendMessage("--- The message has been delivered to the GM team. ---");

        return true;
    } catch (...) {

    }

    return false;
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

    cp->sendMessage(tstring);
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

    cp->sendMessage(tstring);
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

// !prison <time> <player>
bool World::player_prison_command(Player *cp, const std::string &timeplayer) {

    int maxtime = 0;

    if (cp->skills.find("imprisoning") != cp->skills.end()) {
        maxtime = cp->skills["imprisoning"].major;
    } else {
        std::cout << "no skill imprisoning." << std::endl;
    }

    bool returnval=false;

    std::cout << cp->name << " prisoning player: " << timeplayer << " maxtime: " << maxtime << std::endl;

    char *tokenize = new char[ timeplayer.length() + 1 ];
    short int jailtime = 0;

    strcpy(tokenize, timeplayer.c_str());
    char *thistoken;

    if ((thistoken = strtok(tokenize, " ")) != NULL) {
        if (ReadField(thistoken, jailtime)) {
            if (((double)jailtime/60.0) > maxtime || jailtime < 1) {
                std::cout << cp->name << " tried to jail for " << jailtime << " minutes but may only jail for " << maxtime << " hours!" << std::endl;
            } else {
                char *tcharp = strtok(NULL, "\\");

                if (tcharp != NULL) {
                    std::string tplayer = tcharp;
                    position warpto;
                    std::stringstream ssx(configOptions["jail_x"]);
                    ssx >> warpto.x;
                    std::stringstream ssy(configOptions["jail_y"]);
                    ssy >> warpto.y;
                    std::stringstream ssz(configOptions["jail_z"]);
                    ssz >> warpto.z;

                    Player *tempPl;
                    tempPl = Players.find(tplayer);

                    if (tempPl == NULL) {
                        TYPE_OF_CHARACTER_ID tid;

                        // convert arg to digit and try again...
                        std::stringstream ss;
                        ss.str(tplayer);
                        ss >> tid;

                        if (tid) {
                            PLAYERVECTOR::iterator playerIterator;

                            for (playerIterator = Players.begin(); playerIterator < Players.end(); ++playerIterator) {
                                if ((*playerIterator)->id == tid) {
                                    tempPl = (*playerIterator);
                                }
                            }

                        }
                    }

                    if (tempPl != NULL) {
                        std::string tmessage = "*** Jailed " + tempPl->name;
                        cp->sendMessage(tmessage);

                        std::cout << cp->name << " jailed player: " << tempPl->name << " for " << jailtime << std::endl;

                        if (jailtime >= 0) {
                            if (jailtime > 0) {
                                tempPl->SetStatus(JAILEDFORTIME);         // Jailed for time
                                tempPl->SetStatusTime(jailtime * 60);     // Jailed for seconds
                                tempPl->SetStatusGM(cp->id);           // Jailed by who
                            } else if (jailtime == 0) {
                                tempPl->SetStatus(JAILED);                // Jailed indefinately
                                tempPl->SetStatusTime(0);                 // Jailed for seconds
                                tempPl->SetStatusGM(cp->id);           // Jailed by who
                            }

                            tmessage = cp->name + " jailed you for " + (jailtime == 0 ? "eternity" : stream_convert<std::string>((short int &) jailtime) + " minutes");
                            tempPl->sendMessage(tmessage);
                            //warpPlayer( tempPl, warpto );
                            tempPl->Warp(warpto);
                            returnval= true;
                        }
                    } else {
                        std::string tmessage = "*** Could not find " + tplayer;
                        std::cout << tmessage << std::endl;
                        cp->sendMessage(tmessage);
                    }
                }
            }
        }
    }


    delete [] tokenize;

    return returnval;
}

