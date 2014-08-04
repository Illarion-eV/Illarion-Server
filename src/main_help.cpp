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


#include <ctime>
#include <string>
#include <sstream>

#include "data/MonsterTable.hpp"
#include "data/ScheduledScriptsTable.hpp"
#include "script/LuaWeaponScript.hpp" //For standard fighting script.
#include "script/LuaLookAtPlayerScript.hpp"
#include "script/LuaLookAtItemScript.hpp"
#include "script/LuaPlayerDeathScript.hpp"
#include "script/LuaDepotScript.hpp"
#include "script/LuaLoginScript.hpp"
#include "script/LuaLogoutScript.hpp"
#include "script/LuaLearnScript.hpp"
#include "Connection.hpp"
#include "netinterface/NetInterface.hpp"
#include "Logger.hpp"
#include "World.hpp"
#include "Config.hpp"
#include "Player.hpp"

#include "main_help.hpp"

// a table with cyclically called scripts
ScheduledScriptsTable *scheduledScripts;

// a table containing monster descriptions
MonsterTable *MonsterDescriptions;

std::shared_ptr<LuaDepotScript>depotScript;
std::shared_ptr<LuaLookAtPlayerScript>lookAtPlayerScript;
std::shared_ptr<LuaLookAtItemScript>lookAtItemScript;
std::shared_ptr<LuaPlayerDeathScript>playerDeathScript;
std::shared_ptr<LuaLoginScript>loginScript;
std::shared_ptr<LuaLogoutScript>logoutScript;
std::shared_ptr<LuaLearnScript>learnScript;
std::shared_ptr<LuaWeaponScript> standardFightingScript;

// break out of the main loop if false
volatile bool running;

void logout_save(Player *who, bool forced, unsigned long int thistime) {
    time_t acttime;
    time(&acttime);

    thistime = acttime - who->lastsavetime;
    who->onlinetime += thistime;

    unsigned int th = thistime / 3600;
    unsigned int tm = (thistime % 3600) / 60;
    unsigned int ts = (thistime % 3600) % 60;

    unsigned int oh = who->onlinetime / 3600;
    unsigned int om = (who->onlinetime % 3600) / 60;
    unsigned int os = (who->onlinetime % 3600) % 60;

    std::stringstream onlinetime;
    onlinetime << " after " << th << "h " << tm << "m " << ts << "s, onlinetime " << oh << "h " << om << "m " << os << "s";

    Logger::info(LogFacility::Player) << (forced?"forced ":"") << "logout: " << who->Connection->getIPAdress() << *who << " on " << ctime(&acttime) << onlinetime.str() << Log::end;
}

void login_save(Player *who) {
    time_t acttime;
    time(&acttime);

    unsigned int oh = who->onlinetime / 3600;
    unsigned int om = (who->onlinetime % 3600) / 60;
    unsigned int os = (who->onlinetime % 3600) % 60;

    std::stringstream onlinetime;
    onlinetime << " onlinetime till now: " << oh << "h " << om << "m " << os << "s";

    Logger::info(LogFacility::Player) << "login of " << *who << " from " << who->Connection->getIPAdress() << " on " << ctime(&acttime) << onlinetime.str() << Log::end;
}

// process commandline arguments
bool checkArguments(int argc, char *argv[]) {
    if (argc == 2) {
        // config file specified on command line
        if (Config::load(std::string(argv[ 1 ]))) {
            Logger::info(LogFacility::Other) << "main: using configfile: " << argv[ 1 ] << Log::end;
            return true;
        } else {
            Logger::error(LogFacility::Other) << "main: error reading configfile: " << argv[ 1 ] << Log::end;
            Logger::error(LogFacility::Other) << "main: USAGE: " << argv[0] << " configfile" << Log::end;
            return false;
        }
    } else {
        Logger::error(LogFacility::Other) << "main: invalid commandline arguments" << Log::end;
        Logger::error(LogFacility::Other) << "main: USAGE: " << argv[0] << " configfile" << Log::end;
        return false;
    }
}

// load item definitions
void loadData() {
    scheduledScripts = new ScheduledScriptsTable();
    MonsterDescriptions = new MonsterTable();

    try {
        auto tmpScript = std::make_shared<LuaWeaponScript>("server.standardfighting");
        standardFightingScript = tmpScript;
    } catch (ScriptException &e) {
        Logger::error(LogFacility::Script) << "Error while loading script: server.standardfighting: " << e.what() << Log::end;
    }

    try {
        auto tmpScript = std::make_shared<LuaLookAtPlayerScript>("server.playerlookat");
        lookAtPlayerScript = tmpScript;
    } catch (ScriptException &e) {
        Logger::error(LogFacility::Script) << "Error while loading script: server.playerlookat: " << e.what() << Log::end;
    }

    try {
        auto tmpScript = std::make_shared<LuaLookAtItemScript>("server.itemlookat");
        lookAtItemScript = tmpScript;
    } catch (ScriptException &e) {
        Logger::error(LogFacility::Script) << "Error while loading script: server.itemlookat: " << e.what() << Log::end;
    }

    try {
        auto tmpScript = std::make_shared<LuaPlayerDeathScript>("server.playerdeath");
        playerDeathScript = tmpScript;
    } catch (ScriptException &e) {
        Logger::error(LogFacility::Script) << "Error while loading script: server.playerdeath: " << e.what() << Log::end;
    }

    try {
        auto tmpScript = std::make_shared<LuaDepotScript>("server.depot");
        depotScript = tmpScript;
    } catch (ScriptException &e) {
        Logger::error(LogFacility::Script) << "Error while loading script: server.depot: " << e.what() << Log::end;
    }

    try {
        auto tmpScript = std::make_shared<LuaLoginScript>("server.login");
        loginScript = tmpScript;
    } catch (ScriptException &e) {
        Logger::error(LogFacility::Script) << "Error while loading script: server.login: " << e.what() << Log::end;
    }

    try {
        auto tmpScript = std::make_shared<LuaLogoutScript>("server.logout");
        logoutScript = tmpScript;
    } catch (ScriptException &e) {
        Logger::error(LogFacility::Script) << "Error while loading script: server.logout: " << e.what() << Log::end;
    }

    try {
        auto tmpScript = std::make_shared<LuaLearnScript>("server.learn");
        learnScript = tmpScript;
    } catch (ScriptException &e) {
        Logger::error(LogFacility::Script) << "Error while loading script: server.learn: " << e.what() << Log::end;
    }
}


////////////////////////////////////////
// signal handling functions
////////////////////////////////////////

struct sigaction act_segv, act_segv_o , act_pipe, act_pipe_o, act_term, act_term_o, act_usr;

// signal handler for SIGTERM
void sig_term(int) {
    Logger::info(LogFacility::Other) << "SIGTERM received!" << Log::end;
    //  ignore signal
    act_term.sa_handler = SIG_IGN;

    if (sigaction(SIGTERM, &act_term, nullptr) < 0) {
        Logger::error(LogFacility::Other) << "SIGTERM: sigaction failed" << Log::end;
    }

    World::get()->allowLogin(false);
    running = false;
}

// signal handler for SIGSEGV
void sig_segv(int) {
    Logger::error(LogFacility::Other) << "SIGSEGV received! Last Script: " << World::get()->currentScript->getFileName() << Log::end;
    // ignore signal
    act_segv.sa_handler = SIG_IGN;

    if (sigaction(SIGSEGV, &act_segv, nullptr) < 0) {
        Logger::error(LogFacility::Other) << "SIGSEGV: sigaction failed" << Log::end;
    }
}

// signal handler for SIGUSR1 - Used to reload maps
void sig_usr(int) {
    Logger::info(LogFacility::World) << "SIGUSR received! Importing new maps." << Log::end;
    act_usr.sa_handler = sig_usr;

    Logger::info(LogFacility::World) << "Disable login and force log out of all players." << Log::end;
    World *world = World::get();
    world->allowLogin(false);
    world->forceLogoutOfAllPlayers();
    world->maps.clear();
    world->load_maps();

    Logger::info(LogFacility::World) << "Saving World..." << Log::end;
    world->Save();
    world->allowLogin(true);
    Logger::info(LogFacility::World) << "Map import finished." << Log::end;

    if (sigaction(SIGUSR1, &act_usr, nullptr) < 0) {
        Logger::error(LogFacility::Other) << "SIGUSR1: sigaction failed" << Log::end;
    }

}

bool init_sighandlers() {

    // ignore all signals while installing signal handlers
    if (sigfillset(&act_pipe.sa_mask) < 0) {
        Logger::error(LogFacility::Other) << "main: sigfillset failed" << Log::end;
        return false;
    }

    // ignore signals
    act_pipe.sa_handler = SIG_IGN;
    act_pipe.sa_flags = SA_RESTART;

    // install signal handlers
    if (sigaction(SIGPIPE, &act_pipe, &act_pipe_o) < 0) {
        Logger::error(LogFacility::Other) << "SIGPIPE: sigaction failed" << Log::end;
        return false;
    }

    if (sigaction(SIGCHLD, &act_pipe, nullptr) < 0) {
        Logger::error(LogFacility::Other) << "SIGCHLD: sigaction failed" << Log::end;
        return false;
    }

    if (sigaction(SIGINT, &act_pipe, nullptr) < 0) {
        Logger::error(LogFacility::Other) << "SIGINT: sigaction failed" << Log::end;
        return false;
    }

    if (sigaction(SIGQUIT, &act_pipe, nullptr) < 0) {
        Logger::error(LogFacility::Other) << "SIGQUIT: sigaction failed" << Log::end;
        return false;
    }

    // ignore all signals while installing signal handlers
    if (sigfillset(&act_term.sa_mask) < 0) {
        Logger::error(LogFacility::Other) << "main: failed to install signal handlers" << Log::end;
        return false;
    }

    act_term.sa_handler = sig_term;
    act_term.sa_flags = SA_RESTART;

    if (sigaction(SIGTERM, &act_term, &act_term_o) < 0) {
        Logger::error(LogFacility::Other) << "SIGTERM: sigaction failed" << Log::end;
        return false;
    }

    if (sigfillset(&act_segv.sa_mask) < 0) {
        Logger::error(LogFacility::Other) << "main: failed to install signal handlers" << Log::end;
        return false;
    }

    act_segv.sa_handler = sig_segv;
    act_segv.sa_flags = SA_RESTART;

    if (sigaction(SIGSEGV, &act_segv, &act_segv_o) < 0) {
        Logger::error(LogFacility::Other) << "SIGSEGV: sigaction failed" << Log::end;
        return false;
    }

    if (sigfillset(&act_usr.sa_mask) < 0) {
        Logger::error(LogFacility::Other) << "main: failed to install signal handlers" << Log::end;
        return false;
    }

    act_usr.sa_handler = sig_usr;
    act_usr.sa_flags = SA_RESTART;

    if (sigaction(SIGUSR1, &act_usr, nullptr) < 0) {
        Logger::error(LogFacility::Other) << "SIGUSR1: sigaction failed" << Log::end;
        return false;
    }

    return true;
}

void reset_sighandlers() {
    sigaction(SIGPIPE, &act_pipe_o, nullptr);
    sigaction(SIGTERM, &act_term_o, nullptr);
    sigaction(SIGSEGV, &act_segv_o, nullptr);
}
