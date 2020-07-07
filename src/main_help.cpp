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

#include "main_help.hpp"

#include "Config.hpp"
#include "Connection.hpp"
#include "Logger.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "data/MonsterTable.hpp"
#include "data/RaceTypeTable.hpp"
#include "data/ScheduledScriptsTable.hpp"
#include "netinterface/NetInterface.hpp"
#include "script/LuaDepotScript.hpp"
#include "script/LuaLearnScript.hpp"
#include "script/LuaLoginScript.hpp"
#include "script/LuaLogoutScript.hpp"
#include "script/LuaLookAtItemScript.hpp"
#include "script/LuaLookAtPlayerScript.hpp"
#include "script/LuaPlayerDeathScript.hpp"
#include "script/LuaWeaponScript.hpp" //For standard fighting script.

#include <ctime>
#include <memory>
#include <sstream>
#include <string>

std::unique_ptr<ScheduledScriptsTable> scheduledScripts;
std::unique_ptr<MonsterTable> monsterDescriptions;
std::unique_ptr<RaceTypeTable> raceTypes;

std::unique_ptr<LuaDepotScript> depotScript;
std::unique_ptr<LuaLookAtPlayerScript> lookAtPlayerScript;
std::unique_ptr<LuaLookAtItemScript> lookAtItemScript;
std::unique_ptr<LuaPlayerDeathScript> playerDeathScript;
std::unique_ptr<LuaLoginScript> loginScript;
std::unique_ptr<LuaLogoutScript> logoutScript;
std::unique_ptr<LuaLearnScript> learnScript;
std::unique_ptr<LuaWeaponScript> standardFightingScript;

// break out of the main loop if false
volatile bool running;

void logout_save(Player *who, bool forced, unsigned long int thistime) {
    time_t acttime = 0;
    time(&acttime);

    thistime = acttime - who->lastsavetime;
    who->onlinetime += thistime;

    static constexpr auto secondsInMinute = 60;
    static constexpr auto secondsInHour = 60 * 60;

    unsigned int th = thistime / secondsInHour;
    unsigned int tm = (thistime % secondsInHour) / secondsInMinute;
    unsigned int ts = (thistime % secondsInHour) % secondsInMinute;

    unsigned int oh = who->onlinetime / secondsInHour;
    unsigned int om = (who->onlinetime % secondsInHour) / secondsInMinute;
    unsigned int os = (who->onlinetime % secondsInHour) % secondsInMinute;

    std::stringstream onlinetime;
    onlinetime << " after " << th << "h " << tm << "m " << ts << "s, onlinetime " << oh << "h " << om << "m " << os
               << "s";

    Logger::info(LogFacility::Player) << (forced ? "forced " : "") << "logout: " << who->Connection->getIPAdress()
                                      << *who << " on " << ctime(&acttime) << onlinetime.str() << Log::end;
}

void login_save(Player *who) {
    time_t acttime = 0;
    time(&acttime);
    static constexpr auto secondsInMinute = 60;
    static constexpr auto secondsInHour = 60 * 60;

    unsigned int oh = who->onlinetime / secondsInHour;
    unsigned int om = (who->onlinetime % secondsInHour) / secondsInMinute;
    unsigned int os = (who->onlinetime % secondsInHour) % secondsInMinute;

    std::stringstream onlinetime;
    onlinetime << " onlinetime till now: " << oh << "h " << om << "m " << os << "s";

    Logger::info(LogFacility::Player) << "login of " << *who << " from " << who->Connection->getIPAdress() << " on "
                                      << ctime(&acttime) << onlinetime.str() << Log::end;
}

// process commandline arguments
auto checkArguments(const std::vector<std::string> &args) -> bool {
    if (args.size() == 2) {
        // config file specified on command line
        if (Config::load(args.at(1))) {
            Logger::info(LogFacility::Other) << "main: using configfile: " << args.at(1) << Log::end;
            return true;
        }

        Logger::error(LogFacility::Other) << "main: error reading configfile: " << args.at(1) << Log::end;

        Logger::error(LogFacility::Other) << "main: USAGE: " << args.at(0) << " configfile" << Log::end;

        return false;
    }

    Logger::error(LogFacility::Other) << "main: invalid commandline arguments" << Log::end;

    Logger::error(LogFacility::Other) << "main: USAGE: " << args.at(0) << " configfile" << Log::end;

    return false;
}

// load item definitions
void loadData() {
    scheduledScripts = std::make_unique<ScheduledScriptsTable>();
    monsterDescriptions = std::make_unique<MonsterTable>();
    raceTypes = std::make_unique<RaceTypeTable>();

    try {
        standardFightingScript = std::make_unique<LuaWeaponScript>("server.standardfighting");
    } catch (ScriptException &e) {
        Logger::error(LogFacility::Script)
                << "Error while loading script: server.standardfighting: " << e.what() << Log::end;
    }

    try {
        lookAtPlayerScript = std::make_unique<LuaLookAtPlayerScript>("server.playerlookat");
    } catch (ScriptException &e) {
        Logger::error(LogFacility::Script)
                << "Error while loading script: server.playerlookat: " << e.what() << Log::end;
    }

    try {
        lookAtItemScript = std::make_unique<LuaLookAtItemScript>("server.itemlookat");
    } catch (ScriptException &e) {
        Logger::error(LogFacility::Script) << "Error while loading script: server.itemlookat: " << e.what() << Log::end;
    }

    try {
        playerDeathScript = std::make_unique<LuaPlayerDeathScript>("server.playerdeath");
    } catch (ScriptException &e) {
        Logger::error(LogFacility::Script)
                << "Error while loading script: server.playerdeath: " << e.what() << Log::end;
    }

    try {
        depotScript = std::make_unique<LuaDepotScript>("server.depot");
    } catch (ScriptException &e) {
        Logger::error(LogFacility::Script) << "Error while loading script: server.depot: " << e.what() << Log::end;
    }

    try {
        loginScript = std::make_unique<LuaLoginScript>("server.login");
    } catch (ScriptException &e) {
        Logger::error(LogFacility::Script) << "Error while loading script: server.login: " << e.what() << Log::end;
    }

    try {
        logoutScript = std::make_unique<LuaLogoutScript>("server.logout");
    } catch (ScriptException &e) {
        Logger::error(LogFacility::Script) << "Error while loading script: server.logout: " << e.what() << Log::end;
    }

    try {
        learnScript = std::make_unique<LuaLearnScript>("server.learn");
    } catch (ScriptException &e) {
        Logger::error(LogFacility::Script) << "Error while loading script: server.learn: " << e.what() << Log::end;
    }
}

////////////////////////////////////////
// signal handling functions
////////////////////////////////////////

struct sigaction act_segv, act_segv_o, act_pipe, act_pipe_o, act_term, act_term_o, act_usr;

// signal handler for SIGTERM
void sig_term(int /*unused*/) {
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
void sig_segv(int /*unused*/) {
    Logger::error(LogFacility::Other) << "SIGSEGV received! Last Script: " << World::get()->currentScript->getFileName()
                                      << Log::end;
    // ignore signal
    act_segv.sa_handler = SIG_IGN;

    if (sigaction(SIGSEGV, &act_segv, nullptr) < 0) {
        Logger::error(LogFacility::Other) << "SIGSEGV: sigaction failed" << Log::end;
    }
}

// signal handler for SIGUSR1 - Used to reload maps
void sig_usr(int /*unused*/) {
    Logger::info(LogFacility::World) << "SIGUSR received! Importing new maps." << Log::end;
    act_usr.sa_handler = sig_usr;

    Logger::info(LogFacility::World) << "Disable login and force log out of all players." << Log::end;
    World *world = World::get();
    world->allowLogin(false);
    world->forceLogoutOfAllPlayers();
    world->import();
    world->allowLogin(true);
    Logger::info(LogFacility::World) << "Map import finished." << Log::end;

    if (sigaction(SIGUSR1, &act_usr, nullptr) < 0) {
        Logger::error(LogFacility::Other) << "SIGUSR1: sigaction failed" << Log::end;
    }
}

auto init_sighandlers() -> bool {
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
